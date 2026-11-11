#include <P2K_SDK_Base.h>
#include <P2K_DL_Keypad.h>
#include <P2K_SUAPI.h>
#include <P2K_AFW_Event.h>
#include <P2K_DL_File_System.h>
#include <P2K_UIS_Ustring_Portable.h>

#include <P2K_EP3_APP_Entry_Main.h>
#include <P2K_EP3_APP_Viewer.h>
#include <P2K_EP3_ELF_Loader.h>

#include <stdlib.h>
#include <stdio.h>

void APP_CALC_MainRegister(void) {

}

void EP3_APP_Viewer_Register(void) {

}

UINT32 DL_KeyQueryKeypadActivity(void) {
	return 0;
}

SYN_RETURN_STATUS_T AFW_CreateInternalQueuedEvPriv(
	AFW_EVENT_CODE_T                   ev_code,
	AFW_AUF_SEQ_NUM_T                  ev_seqn,
	AFW_EV_ROUTE_ID_T                  ev_rtid,
	AFW_EVENT_CATG_T                   ev_catg,
	AFW_DATA_LOG_APP_ID_T              log_id,
	AFW_EVENT_DATA_T *                 ev_data,
	AFW_EV_BUF_FLAG_T                  ev_bflag,
	AFW_EV_BUF_SIZE_T                  ev_bsize,
	AFW_EV_BUF_T *                     ev_auxd,
	AFW_SVC_EV_TYPE_T                  svc_type
) {
	UNUSED(ev_code);
	UNUSED(ev_seqn);
	UNUSED(ev_rtid);
	UNUSED(ev_catg);
	UNUSED(log_id);
	UNUSED(ev_data);
	UNUSED(ev_bflag);
	UNUSED(ev_bsize);
	UNUSED(ev_auxd);
	UNUSED(svc_type);

	return SYN_SUCCESS;
}

void suRegisterName(const char *name, UINT32 handle, SU_RET_STATUS *err) {
	UNUSED(name);
	UNUSED(handle);

	*err = SU_OK;
}

UINT32 suFindName(const char *name, SU_TIME timeout, SU_RET_STATUS *err) {
	UNUSED(name);
	UNUSED(timeout);

	*err = SU_OK;

	return 0x00000000;
}

void *suAllocMem(SU_SIZE buffer_size, SU_RET_STATUS *error_ptr) {
	void *buffer = malloc(buffer_size);
	if (buffer == NULL) {
		*error_ptr = SU_EFULL;
	} else {
		*error_ptr = SU_OK;
	}
	return buffer;
}

void suFreeMem(void *buffer_ptr) {
	free(buffer_ptr);
}

#define MAX_GLOBAL_FILES               (64)

static FILE *g_files[MAX_GLOBAL_FILES];
static DL_FS_HANDLE_T g_files_cursor = 0x0001;

DL_FS_SIZE_T DL_FsSGetFileSize(const WCHAR *file_uri, DL_FS_OWNER_T owner_id) {
	UNUSED(owner_id);

	char uri[DL_FS_MAX_URI_NAME_LENGTH];

	PORTABLE_u_utoa(file_uri, uri);

	FILE *fp = fopen(uri, "rb");
	if (fp == NULL) {
		return 0UL;
	}

	if (fseek(fp, 0, SEEK_END) != 0) {
		fclose(fp);
		return 0UL;
	}

	long size = ftell(fp);
	fclose(fp);
	return size;
}

DL_FS_HANDLE_T DL_FsOpenFile(
	const WCHAR *                      file_uri,
	DL_FS_OPEN_MODE_T                  open_mode,
	DL_FS_OWNER_T                      owner_id
) {
	UNUSED(owner_id);

	char uri[DL_FS_MAX_URI_NAME_LENGTH];
	PORTABLE_u_utoa(file_uri, uri);

	const char *mode;

	switch (open_mode) {
		default:
		case DL_FS_READ_MODE:
			mode = "rb"; break;
		case DL_FS_WRITE_MODE:
			mode = "wb"; break;
		case DL_FS_WRITE_EXIST_MODE:
			mode = "wbx"; break;
		case DL_FS_APPEND_MODE:
			mode = "ab"; break;
		case DL_FS_READ_PLUS_MODE:
			mode = "r+b"; break;
		case DL_FS_WRITE_PLUS_MODE:
			mode = "w+b"; break;
		case DL_FS_WRITE_EXIST_PLUS_MODE:
			mode = "w+bx"; break;
		case DL_FS_APPEND_PLUS_MODE:
			mode = "a+b"; break;
	}

	FILE *fp = fopen(uri, mode);
	if (fp == NULL) {
		return DL_FS_HANDLE_INVALID;
	}

	g_files[g_files_cursor] = fp;
	g_files_cursor++;
	if (g_files_cursor >= MAX_GLOBAL_FILES) {
		g_files_cursor = 0x0001;
	}

	return g_files_cursor;
}

DL_FS_RESULT_T DL_FsCloseFile(DL_FS_HANDLE_T file_handle) {
	if (g_files[g_files_cursor]) {
		if (!fclose(g_files[file_handle])) {
			return DL_FS_RESULT_FAIL;
		}
		g_files[file_handle] = NULL;
	}
	return DL_FS_RESULT_SUCCESS;
}

DL_FS_RESULT_T DL_FsReadFile(
	void *                             buffer,
	DL_FS_SIZE_T                       element_size,
	DL_FS_COUNT_T                      count,
	DL_FS_HANDLE_T                     file_handle,
	DL_FS_COUNT_T *                    elements_read
) {
	const size_t elements = fread(buffer, element_size, count, g_files[file_handle]);
	if (elements != count) {
		return DL_FS_RESULT_FAIL;
	}

	*elements_read = elements;
	return DL_FS_RESULT_SUCCESS;
}

DL_FS_RESULT_T DL_FsWriteFile(
	void *                             buffer,
	DL_FS_SIZE_T                       element_size,
	DL_FS_COUNT_T                      count,
	DL_FS_HANDLE_T                     file_handle,
	DL_FS_COUNT_T *                    elements_written
) {
	const size_t elements = fwrite(buffer, element_size, count, g_files[file_handle]);
	if (elements != count) {
		return DL_FS_RESULT_FAIL;
	}

	*elements_written = elements;
	return DL_FS_RESULT_SUCCESS;
}

DL_FS_RESULT_T DL_FsFSeekFile(
	DL_FS_HANDLE_T                     file_handle,
	DL_FS_SEEK_OFFSET_T                offset,
	DL_FS_SEEK_FROM_T                  seek_from
) {
	int seek_mode;
	switch (seek_from) {
		default:
		case DL_FS_SEEK_SET:
			seek_mode = SEEK_SET; break;
		case DL_FS_SEEK_CUR:
			seek_mode = SEEK_CUR; break;
		case DL_FS_SEEK_END:
			seek_mode = SEEK_END; break;
	}

	int res = fseek(g_files[file_handle], offset, seek_mode);
	if (res != 0) {
		return DL_FS_RESULT_FAIL;
	}

	return DL_FS_RESULT_SUCCESS;
}

const char *EP3_phone_str = "C330";
const char *EP3_firmware_str = "C330_TA02_G_06.03.25R_elf";

UINT32 EP3_phone;
UINT32 EP3_firmware;

int main(int argc, char *argv[]) {
	UNUSED(argc);
	UNUSED(argv);

	EP3_phone = (UINT32) &EP3_phone_str[0];
	EP3_firmware = (UINT32) &EP3_phone_str[0];

	EP3_ELF_Loader_MainRegister();

	return 0;
}
