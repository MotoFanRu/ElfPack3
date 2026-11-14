#include <P2K_SDK_Base.h>
#include <P2K_Logger.h>
#include <P2K_SUAPI.h>

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
	 * Для проверки правильной работы "va_args" мы вызываем некоторые функции вывода логов.
	 */
	L("[EP3_AGT]: %s\n", "Hello Moto!");
	L("[EP3_AGT]: %s %s\n", "Phone:", EP3_API_Get_Phone());
	L("[EP3_AGT]: %s %s\n", "Firmware:", EP3_API_Get_Firmware());
	L("[EP3_AGT]: %s %s\n", "Version:", EP3_API_Get_Version());
	D("[EP3_AGT]: %s\n", "Debug build.");

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Мы подменяем адрес функции `APP_CALC_MainRegister()` внутри таблицы `app_pra_registration_table[]` на
	 * адрес функции `EP3_Agent_Main_Register_Entry()` и поэтому должны обязательно вызывать оригинальную
	 * функцию регистрации приложения будь то это калькулятор или что-либо ещё.
	 */
	APP_CALC_MainRegister();

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Регистрируем приложение для просмотра сообщений на экране телефона.
	 */
	EP3_APP_Viewer_Register();

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Проверяем, нажата ли кнопка "0" на телефоне во время включения.
	 * Если кнопки нажата, то ничего не запускаем.
	 */
#if 0 // TODO: Implement it with research.
	if (EP3_API_Key_State() == P2K_KEY_0) {
		V("EP3 Agent", "[EP3_AGT]: Key '0' was pressed, loading of '%s' disabled.", EP3_ELF_LDR_NAME);
		return;
	}
#endif

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Ищем загружаемый в RAM компонент в файловой системе телефона.
	 * Если не нашли, выводим в лог ошибку и ничего не делаем.
	 */
	WCHAR file_path[PATH_MAX_COMPONENT];
	if (EP3_Find_Component(EP3_ELF_LDR_NAME, file_path) != RESULT_OK) {
		L("[EP3_AGT]: Not found: %s\n", EP3_ELF_LDR_NAME);
		return;
	}

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Регистрируем в SUAPI функцию BIN Loader'а как API.
	 * Это позволит использовать её глобально без внесения в библиотеки.
	 */
	SU_RET_STATUS status;
	const UINTPTR binloader_func = (UINTPTR) &EP3_API_BIN_Load;
	suRegisterName(BINLOADER_FUNC_NAME, (UINT32) binloader_func, &status);
	if (status != SU_OK) {
		D("[EP3_AGT]: suRegisterName() Err, res=%d, name=%s, func=0x%08X\n", status, BINLOADER_FUNC_NAME, binloader_func);
		return;
	}
	D("[EP3_AGT]: suRegisterName() OK, name=%s, func=0x%08X\n", BINLOADER_FUNC_NAME, binloader_func);

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Получаем зарегистрированный адрес функции BIN Loader'а из SUAPI.
	 */
	const EP3_BIN_LOAD_T EP3_API_BIN_Load_Func = (EP3_BIN_LOAD_T) suFindName(BINLOADER_FUNC_NAME, SU_NOWAIT, &status);
	if (status != SU_OK) {
		D("[EP3_AGT]: suFindName() Err, res=%d, name=%s\n", status, BINLOADER_FUNC_NAME);
		return;
	}
	if (EP3_API_BIN_Load_Func == NULL) {
		D("[EP3_AGT]: %s\n", "suFindName() Err, func=NULL");
		return;
	}

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Расставляем параметры, загружаем в RAM компонент и запускаем.
	 *
	 * Следует обратить внимание, что в случае если компиляция была выполнена
	 * в режиме `-mthumb` вместо `-marm` мы должны установить бит, который
	 * сделает загружаемы адрес нечётным.
	 */
	UINTPTR load_bin_addr = NULL_ADDR; /* 0x00000000 */
#if defined(FTR_LOAD_TO_ADDR)
	load_bin_addr = (UINTPTR) FTR_LOAD_TO_ADDR;
#endif /* FTR_LOAD_TO_ADDR */

	BOOL set_bit = FALSE;
#if defined(FTR_THUMB_MODE)
	set_bit = TRUE;
#endif /* FTR_THUMB_MODE */

	EP3_API_BIN_Load_Func(file_path, NULL, load_bin_addr, set_bit, FALSE);
}
