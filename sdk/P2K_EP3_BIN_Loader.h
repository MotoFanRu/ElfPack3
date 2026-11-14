#ifndef P2K_EP3_BIN_LOADER_H
#define P2K_EP3_BIN_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define BINLOADER_FUNC_NAME            "ep3/bin"

/*
 * EXL, 15-Nov-2026:
 *
 * Структура аргументов, которые передаются в функцию загрузки и исполнения
 * бинарных образов.
 *
 *     file_path    - Указатель на путь до файла бинарного образа.
 *     wargs        - Указатель на массив различных аргументов.
 *     load_addr    - Адрес загрузки, который может быть перезаписан конфигом,
 *                    если он лежит рядом, либо аллокатором памяти, если
 *                    этот аргумент изначально равен `NULL_ADDR`. Выделенную
 *                    память нужно будет освобождать в приложении вручную.
 *     set_addr_bit - Флаг, указывающий нужно ли делать адрес загрузки нечётным.
 *                    Это необходимо для корректной работы режима Thumb в
 *                    процессорах семейства ARM.
 *     save_mem     - Флаг, указывающий на то, нужно ли сохранять содержимое
 *                    блока памяти перед загрузкой туда бинарного образа.
 *                    Восстанавливать блок нужно в самом приложении вручную.
 */
typedef struct tagBIN_LOADER_ARGS_T {
	const WCHAR *file_path;
	const WCHAR *wargs;
	const UINTPTR *load_addr;
	BOOL set_addr_bit;
	BOOL save_mem;
} BIN_LOADER_ARGS_T;

/*
 * EXL, 15-Nov-2026:
 *
 * Структура аргументов, которые передаются в точку входа бинарных образов.
 *
 *     file_path           - Указатель на путь до файла бинарного образа.
 *     wargs               - Указатель на массив различных аргументов.
 *     dir_path            - Указатель на путь до директории с бинарным образом.
 *     load_addr           - Оригинальный адрес загрузки бинарного образа.
 *                           Именно его нужно использовать при освобождении
 *                           памяти вручную если он не фиксированный.
 *     load_addr_adjusted  - Выровненный адрес с применённым битом если нужно.
 *     save_addr           - Адрес сохранённого клонированного блока памяти.
 *                           Если `NULL_ADDR`, то блок не был выделен.
 *     file_size           - Размер файла, необходимо знать, чтобы восстановить
 *                           скопированный блок памяти обратно.
 *     save_addr_allocated - Флаг, указывающий на то, что нужно освободить
 *                           память сохранённого клонированного блока.
 *     load_addr_allocated - Флаг, указывающий на то, что нужно освободить
 *                           память выделенную под бинарный образ.
 */
typedef struct tagBIN_ARGS_T {
	const WCHAR *file_path;
	const WCHAR *wargs;
	const WCHAR *dir_path;
	const UINTPTR *load_addr;
	const UINTPTR *load_addr_adjusted;
	const UINTPTR *save_addr;
	UINT32 file_size;
	BOOL save_addr_allocated;
	BOOL load_addr_allocated;
} BIN_ARGS_T;

/*
 * EXL, 15-Nov-2026:
 *
 * Функция загрузки бинарного образа в память и переход к его исполнению.
 */
extern STATUS EP3_API_BIN_Load(const BIN_LOADER_ARGS_T *args);
typedef STATUS (* EP3_API_BIN_LOAD_T)(const BIN_LOADER_ARGS_T *args);

/*
 * EXL, 15-Nov-2026:
 *
 * Функция клонирования блока памяти. Аллоцирует новый блок памяти.
 * Копирует туда память из источника нужным размером и возвращает
 * адрес на новый блок.
 */
extern void *EP3_API_BIN_Clone_Memory(const void *src_addr, SIZE_T size);

/*
 * EXL, 15-Nov-2026:
 *
 * Функция восстановления блока памяти. Копирует память блока `src_addr` в
 * блок `dst_addr`, освобождает блок `src_addr` и возвращает адрес на
 * восстановленный блок памяти.
 */
extern void *EP3_API_BIN_Restore_Memory(void *dst_addr, void *src_addr, SIZE_T size);

/*
 * EXL, 15-Nov-2026:
 *
 * Функция точки входа бинарного образа, при возвращении результата
 * исполнения `RESULT_OK_CLEAN_MEMORY` автоматически освобождает память и
 * восстанавливает клонированные блоки.
 */
typedef STATUS (* EP3_BIN_LOADER_ROUTINE_T)(const BIN_ARGS_T *args);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_BIN_LOADER_H */
