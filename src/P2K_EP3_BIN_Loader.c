#include <P2K_SDK_Base.h>
#include <P2K_C_Lib.h>
#include <P2K_DL_File_System.h>

#include <P2K_Logger.h>

#include <P2K_EP3_Base.h>
#include <P2K_EP3_Memory.h>
#include <P2K_EP3_Config.h>
#include <P2K_EP3_Image_Manager.h>
#include <P2K_EP3_BIN_Loader.h>

#include "P2K_UIS_Ustring_Portable.h"

extern STATUS EP3_API_BIN_Load(const BIN_LOADER_ARGS_T *args) {
	P();

	if (EMPTY_STR(args->file_path)) {
		D("[EP3 BIN E]: %s\n", "File path is NULL or empty");
		return RESULT_FAIL_WRONG_ARG;
	}

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Для проверки существования файла имеет смысл использовать функцию
	 * прошивки `DL_FsSGetFileSize()` вместо `DL_FsGetFileSize()` которая
	 * требует сначала открыть файл. Это позволит немного упростить код.
	 */
	const DL_FS_SIZE_T file_size =
		DL_FsSGetFileSize(args->file_path, DL_FS_OWNER_RESERVED);
	if (file_size < FILE_SIZE_TOO_SMALL) {
		D("[EP3 BIN E]: %s\n", "File not exist or too small");
		DU("[EP3 BIN E]:  file_path=%s\n", args->file_path);
		D("[EP3 BIN E]:  file_size=%d\n", file_size);
		return RESULT_FAIL_FILE_NOT_FOUND;
	}

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Пробуем открыть файлик для последующего чтения.
	 */
	const DL_FS_HANDLE_T file_hdl =
		DL_FsOpenFile(args->file_path, DL_FS_READ_MODE, DL_FS_OWNER_RESERVED);
	if (file_hdl == DL_FS_HANDLE_INVALID) {
		D("[EP3 BIN E]: %s\n", "Cannot open file");
		DU("[EP3 BIN E]:  file_path=%s\n", args->file_path);
		return RESULT_FAIL_FILE_CANT_OPEN;
	}

	UINTPTR load_addr = (UINTPTR) args->load_addr;
	BOOL save_mem = args->save_mem;

	/* EXL, 15-Nov-2026:
	 *
	 * Читаем и обновляем значения двух параметров из конфигурационного файла
	 * вида args->file_path + '.cfg', если он имеется рядом с бинарным файлом.
	 */
	EP3_Image_Cfg_T image_cfg;
	if (EP3_Image_Config_Load(args->file_path, &image_cfg) == RESULT_OK) {
		load_addr = image_cfg.load_addr;
		save_mem = (BOOL) image_cfg.save_mem;
		D("[EP3 BIN I]: %s\n", "Config was loaded");
		D("[EP3 BIN I]:  load_addr=0x%08X\n", load_addr);
		D("[EP3 BIN I]:  save_mem=%d\n", save_mem);
	} else {
		D("[EP3 BIN I]: %s\n", "Config not found");
	}

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Ранее официальный компилятор M-CORE GCC не умел генерировать
	 * позиционно-независимый код с помощью флагов `-fPIC`, `-fPIE` и `-pie`,
	 * поэтому мы не могли использовать случайный адрес в памяти, который
	 * выдавал нам аллокатор и поэтому были вынуждены использовать
	 * фиксированный адрес в памяти для загрузки программы. К счастью,
	 * в новом форке компилятора M-CORE GCC появилась возможность генерации
	 * позиционно-независимого кода.
	 *
	 * Если адрес загрузки задан `NULL_ADDR`, мы используем аллоцирование.
	 */
	BOOL bin_memory_allocated = FALSE;
	UINTPTR original_load_addr = NULL_ADDR;
	if (load_addr == NULL_ADDR) {
		/*
		 * EXL, 15-Nov-2026:
		 *
		 * На всякий случай к размеру загружаемого бинарного образа
		 * программы добавим 4 байта, чтобы адрес потом можно было
		 * безопасно выровнять.
		 */
		load_addr = (UINTPTR) EP3_Memory_Alloc(file_size + 4);
		if (load_addr == NULL_ADDR) {
			DL_FsCloseFile(file_hdl);
			return RESULT_FAIL_CANT_ALLOC;
		}

		/*
		 * EXL, 15-Nov-2026:
		 *
		 * Выравниваем адрес загрузки бинарного образа таким образом,
		 * чтобы он был кратен 4 байтам. Но при этом сохраняем
		 * оригинальный адрес чтобы в случае чего иметь возможность
		 * освободить память.
		 */
		original_load_addr = load_addr;
		load_addr = ((load_addr + 3) & ~3UL);
		bin_memory_allocated = TRUE;
	}

	BOOL clone_memory_allocated = FALSE;
	UINTPTR saved_mem_addr = NULL_ADDR;
	if (save_mem) {
		saved_mem_addr =
			(UINTPTR) EP3_API_BIN_Clone_Memory((void *) load_addr, file_size);
		if (saved_mem_addr == NULL_ADDR) {
			L("[EP3 BIN E]: %s\n", "Cannot clone memory");
			L("[EP3 BIN E]:  load_addr=0x%08X\n", (UINTPTR) load_addr);
			L("[EP3 BIN E]:  file_size=%d\n", file_size);
			DL_FsCloseFile(file_hdl);
			if (bin_memory_allocated) {
				EP3_Memory_Free((void *) original_load_addr);
			}
			return RESULT_FAIL_CANT_CLONE;
		}
		L("[EP3 BIN E]: %s\n", "Memory cloned");
		L("[EP3 BIN E]:  load_addr=0x%08X\n", load_addr);
		L("[EP3 BIN E]:  saved_mem_addr=0x%08X\n", saved_mem_addr);
		L("[EP3 BIN E]:  file_size=%d\n", file_size);
		clone_memory_allocated = TRUE;
	}

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Прочитаем файл и загрузим его внутрь выданной памяти.
	 */
	DL_FS_COUNT_T elements_read;
	const DL_FS_RESULT_T result = DL_FsReadFile(
		(void *) load_addr, file_size, 1, file_hdl, &elements_read
	);
	if ((result != DL_FS_RESULT_SUCCESS) || (elements_read != 1)) {
		D("[EP3 BIN E]: %s\n", "Failed to read file into memory");
		D("[EP3 BIN E]:  result=%d\n", result);
		DU("[EP3 BIN E]:  file_path=%s\n", args->file_path);
		D("[EP3 BIN E]:  load_addr=0x%08X\n", load_addr);
		D("[EP3 BIN E]:  elements_read=%d\n", elements_read);
		DL_FsCloseFile(file_hdl);
		if (bin_memory_allocated) {
			EP3_Memory_Free((void *) original_load_addr);
		}
		if (clone_memory_allocated) {
			EP3_Memory_Free((void *) saved_mem_addr);
		}
		return RESULT_FAIL_FILE_CANT_READ;
	}
	L("[EP3 BIN I]: %s\n", "File loaded to memory");
	L("[EP3 BIN I]:  file_size=%d\n", file_size);
	LU("[EP3 BIN I]:  file_path=%s\n", args->file_path);
	L("[EP3 BIN I]:  load_addr=0x%08X\n", load_addr);
	DL_FsCloseFile(file_hdl);

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Сделаем адрес загрузки нечётным, если это нужно. Это позволит
	 * переключить режим процессора из ARM в Thumb. В случае использования
	 * ARM вместо Thumb или M-CORE этот бит выставлять не имеет смысла.
	 */
	if (args->set_addr_bit) {
		load_addr = load_addr | 1;
	}

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Заполним новую структуру аргументами и прыгаем на адрес для исполнения
	 * загруженного кода и затем проверяем код его возврата.
	 *
	 * Внимание, освобождение выделенной памяти при отсутствии фиксированного
	 * адреса загрузки при котором будет аллоцирование памяти и будет выдан
	 * случайный адрес в RAM, так же как и освобождение и возврат клонированной
	 * памяти на место должно производиться внутри самого приложения,
	 * поскольку если это приложение запущено в рамках P2K/Synergy/AFW
	 * фреймворков, его исполнение нелинейно и событийно.
	 *
	 * Проверка на уже запущенный на телефоне код тоже должна выполняться
	 * на стороне приложения через регистрацию имён в SUAPI или токен
	 * приложения AFW.
	 */
	WCHAR *dir_path =
		(WCHAR *) EP3_Memory_Alloc(DL_FS_MAX_PATH_NAME_LENGTH * sizeof(WCHAR));
	D("[EP3 BIN I]: Alloc dir_path=0x%08X\n", dir_path);
	PORTABLE_u_dirname(args->file_path, dir_path);

	BIN_ARGS_T bin_args;
	bin_args.file_path = args->file_path;
	bin_args.wargs = args->wargs;
	bin_args.dir_path = dir_path;
	bin_args.load_addr = (UINTPTR *) original_load_addr;
	bin_args.load_addr_adjusted = (UINTPTR *) load_addr;
	bin_args.save_addr = (UINTPTR *) saved_mem_addr;
	bin_args.file_size = file_size;
	bin_args.load_addr_allocated = bin_memory_allocated;
	bin_args.save_addr_allocated = clone_memory_allocated;

	L("[EP3 BIN I]: Jump to load_addr=0x%08X\n", (UINTPTR) load_addr);
	const EP3_BIN_LOADER_ROUTINE_T bin_loader =
		(EP3_BIN_LOADER_ROUTINE_T) ((UINTPTR) load_addr);
	const STATUS status = bin_loader(&bin_args);
	L("[EP3 BIN I]: Returned status=%d\n", status);

	D("[EP3 BIN I]: Freed dir_path=0x%08X\n", dir_path);
	EP3_Memory_Free(dir_path);

	if (status == RESULT_OK_CLEAN_MEMORY) {
		if (bin_memory_allocated) {
			D("[EP3 BIN I]: %s\n", "Will freed binary");
			D("[EP3 BIN I]: load_addr=0x%08X\n", (UINTPTR) bin_args.load_addr);
			EP3_Memory_Free((void *) bin_args.load_addr);
		}
		if (clone_memory_allocated) {
			D("[EP3 BIN I]: %s\n", "Will restore memory");
			D("[EP3 BIN I]: load_addr=0x%08X\n", (UINTPTR) bin_args.load_addr);
			D("[EP3 BIN I]: save_addr=0x%08X\n", (UINTPTR) bin_args.save_addr);
			D("[EP3 BIN I]: file_size=%d\n", file_size);
			EP3_API_BIN_Restore_Memory(
				(void *) bin_args.load_addr,
				(void *) bin_args.save_addr,
				file_size
			);
		}
	} else {
		D("[EP3 BIN I]: %s\n", "No additional actions required");
	}

	return RESULT_OK;
}

void *EP3_API_BIN_Clone_Memory(const void *src_addr, SIZE_T size) {
	P();

	if (src_addr == NULL) {
		return NULL;
	}
	if (size < MEMORY_SIZE_TOO_SMALL) {
		return NULL;
	}

	void *dst_addr = EP3_Memory_Alloc(size);
	if (dst_addr == NULL) {
		return NULL;
	}

	D("[EP3 BIN I]: %s\n", "Cloning memory...");
	D("[EP3 BIN I]: dst_addr=0x%08X\n", dst_addr);
	D("[EP3 BIN I]: src_addr=0x%08X\n", src_addr);
	D("[EP3 BIN I]: size=%d\n", size);
	memcpy(dst_addr, src_addr, size);

	return dst_addr;
}

void *EP3_API_BIN_Restore_Memory(
	void *dst_addr,
	void *src_addr,
	SIZE_T size
) {
	P();

	if (dst_addr == NULL) {
		return NULL;
	}
	if (src_addr == NULL_ADDR) {
		return NULL;
	}
	if (size <= MEMORY_SIZE_TOO_SMALL) {
		return NULL_ADDR;
	}

	D("[EP3 BIN I]: %s\n", "Restoring memory...");
	D("[EP3 BIN I]: dst_addr=0x%08X\n", dst_addr);
	D("[EP3 BIN I]: src_addr=0x%08X\n", src_addr);
	D("[EP3 BIN I]: size=%d\n", size);

	memcpy(dst_addr, src_addr, size);

	EP3_Memory_Free(src_addr);

	return dst_addr;
}
