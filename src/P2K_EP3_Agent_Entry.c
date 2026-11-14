#include <P2K_SDK_Base.h>
#include <P2K_SUAPI.h>

#include <P2K_Logger.h>

#include <P2K_EP3_Base.h>
#include <P2K_EP3_PFW_Version.h>
#include <P2K_EP3_APP_Viewer.h>
#include <P2K_EP3_Component.h>
#include <P2K_EP3_BIN_Loader.h>
#include <P2K_EP3_Agent_Entry.h>

__attribute__((used, section(".text.ep3.bin_entry_point")))
void EP3_Agent_Main_Register_Entry(void) {
	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Для проверки правильной работы "va_args" мы вызываем некоторые функции
	 * для вывода логов через определённый логгер.
	 *
	 * Базовые функции:
	 *     P() (DEBUG) - Вывод отладочного лога, если -DFTR_DEBUG активирован.
	 *     L() (LOG)   - Вывод обычного лога.
	 *     D() (DEBUG) - Вывод отладочного лога, если -DFTR_DEBUG активирован.
	 *     V() (VIEW)  - Вывод сообщения на экран телефона.
	 */
	P();
	L("[EP3 AGT I]: %s\n", "Hello Moto");
	L("[EP3 AGT I]: %s %s\n", "Phone:", EP3_API_Get_Phone());
	L("[EP3 AGT I]: %s %s\n", "Firmware:", EP3_API_Get_Firmware());
	L("[EP3 AGT I]: %s %s\n", "Version:", EP3_API_Get_Version());
	D("[EP3 AGT D]: %s\n", "Debug");

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Мы подменяем адрес функции `APP_CALC_MainRegister()` внутри
	 * таблицы `app_pra_registration_table[]` на адрес
	 * функции `EP3_Agent_Main_Register_Entry()` и поэтому должны
	 * обязательно вызывать оригинальную функцию регистрации приложения
	 * будь то это калькулятор или что-либо ещё.
	 */
	APP_CALC_MainRegister();

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Регистрируем простое приложение для просмотра сообщений на
	 * дисплее телефона.
	 */
	EP3_APP_Viewer_Register();

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Проверяем, нажата ли кнопка "0" на телефоне во время включения.
	 * Если кнопки нажата, то ничего не запускаем и выводим сообщение.
	 */
// TODO: Implement it with some keyboard port research.
#if 0
	if (EP3_API_Key_State() == P2K_KEY_0) {
		V(
			"[EP3 AGT V]:",
			"The '0' key was pressed during startup, '%s' is disabled.",
			EP3_ELF_LDR_NAME
		);
		return;
	}
#endif

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Ищем нужный компонент в файловой системе телефона. Если ничего не нашли,
	 * просто выводим ошибку в лог вместо дисплея телефона и ничего не делаем.
	 */
	WCHAR file_path[PATH_MAX_COMPONENT];
	if (EP3_Find_Component(EP3_ELF_LDR_NAME, file_path) != RESULT_OK) {
		L("[EP3 AGT E]: Not found: %s", EP3_ELF_LDR_NAME);
		return;
	}

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Регистрируем в SUAPI главную функцию бинарного загрузчика как API.
	 * Это позволит использовать её глобально без внесения в библиотеки.
	 */
	SU_RET_STATUS status;
	const UINTPTR func_addr = (UINTPTR) &EP3_API_BIN_Load;
	suRegisterName(BINLOADER_FUNC_NAME, (UINT32) func_addr, &status);
	if (status != SU_OK) {
		D("[EP3 AGT E]: %s\n", "Cannot register " BINLOADER_FUNC_NAME);
		D("[EP3 AGT E]: %s\n", "suRegisterName()");
		D("[EP3 AGT E]:  status=%d\n", status);
		D("[EP3 AGT E]:  func_addr=0x%08X\n", (UINTPTR) func_addr);
		return;
	}
	D("[EP3 AGT D]: %s\n", "Registered " BINLOADER_FUNC_NAME);
	D("[EP3 AGT D]: %s\n", "suRegisterName()");
	D("[EP3 AGT D]:  status=%d\n", status);
	D("[EP3 AGT D]:  func_addr=0x%08X\n", (UINTPTR) func_addr);

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Получаем зарегистрированный адрес функции бинарного загрузчика из SUAPI.
	 * Просто для проверки, что система регистрации функций в SUAPI работает.
	 */
	const EP3_API_BIN_LOAD_T bin_load_func = (EP3_API_BIN_LOAD_T) suFindName(
		BINLOADER_FUNC_NAME, SU_NOWAIT, &status
	);
	if (status != SU_OK) {
		D("[EP3 AGT E]: %s\n", "Cannot find " BINLOADER_FUNC_NAME);
		D("[EP3 AGT E]: %s\n", "suFindName()");
		D("[EP3 AGT E]:  status=%d\n", status);
		return;
	}
	if (bin_load_func == NULL) {
		D("[EP3 AGT E]: func_hdl=%s\n", "NULL");
		return;
	}
	D("[EP3 AGT D]: %s\n", "Found " BINLOADER_FUNC_NAME);
	D("[EP3 AGT D]: %s\n", "suFindName()");
	D("[EP3 AGT D]:  status=%d\n", status);
	D("[EP3 AGT D]:  bin_load_func=0x%08X\n", (UINTPTR) bin_load_func);

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Расставляем параметры, загружаем в RAM компонент и запускаем.
	 *
	 * Следует обратить внимание, что в случае если компиляция была выполнена
	 * в режиме `-mthumb` вместо `-marm` мы должны установить бит, который
	 * сделает загружаемый адрес нечётным для Thumb.
	 */
	BIN_LOADER_ARGS_T args = { 0 };
	args.file_path = file_path;

#if defined(FTR_LOAD_TO_ADDR)
	args.load_addr = (UINTPTR *) FTR_LOAD_TO_ADDR;
#endif /* FTR_LOAD_TO_ADDR */

#if defined(FTR_THUMB_MODE)
	args.set_addr_bit  = TRUE;
#endif /* FTR_THUMB_MODE */

	/* EP3_API_BIN_Load() */
	bin_load_func(&args);
}
