#ifndef P2K_DL_FILE_SYSTEM_H
#define P2K_DL_FILE_SYSTEM_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define DL_FS_SIZE_INVALID             (0)
#define DL_FS_HANDLE_INVALID           (0xFFFF)

typedef UINT16                         DL_FS_HANDLE_T;
typedef UINT32                         DL_FS_SIZE_T;
typedef UINT32                         DL_FS_COUNT_T;
typedef INT32                          DL_FS_SEEK_OFFSET_T;

enum tagDL_FS_OPEN_MODE_T {
	DL_FS_READ_MODE = 0,               /* Read-only at current position. */
	DL_FS_WRITE_MODE,                  /* Write-only truncate file. */
	DL_FS_WRITE_EXIST_MODE,            /* Write-only, if file does not exist. */
	DL_FS_APPEND_MODE,                 /* Append; open or create file for writing at end-of-file. */
	DL_FS_READ_PLUS_MODE,              /* Read and write at current position. */
	DL_FS_WRITE_PLUS_MODE,             /* Read and write, truncate file. */
	DL_FS_WRITE_EXIST_PLUS_MODE,       /* Read and write, if file does not exist. */
	DL_FS_APPEND_PLUS_MODE             /* Append; open or create file for reading or writing, writing at end-of-file. */
};
typedef UINT8                          DL_FS_OPEN_MODE_T;

enum tagDL_FS_OWNER_T {
	DL_FS_OWNER_RESERVED = 0,
	DL_FS_OWNER_INVALID                /* Must remain last entry. */
};
typedef UINT8                          DL_FS_OWNER_T;

enum tagDL_FS_RESULT_T {
	DL_FS_RESULT_SUCCESS = 0,          /* 0 is OK, >= 1 is FAIL. This enum is not complete. */
	DL_FS_RESULT_FAIL
};
typedef UINT8                          DL_FS_RESULT_T;

enum tagDL_FS_SEEK_FROM_ENUM_T {
	DL_FS_SEEK_SET = 0,                /* The beginning of the file. */
	DL_FS_SEEK_CUR,                    /* The current position. */
	DL_FS_SEEK_END                     /* The end of the file. */
};
typedef UINT8                          DL_FS_SEEK_FROM_T;

/*
 * This function is called by application to open a file.
 *
 * The valid return value for DL_FS_HANDLE_T is any number between 0 and 255 on success,
 * and DL_FS_HANDLE_INVALID on failure.
 */
extern DL_FS_HANDLE_T DL_FsOpenFile(
	const WCHAR *                      file_uri,
	const DL_FS_OPEN_MODE_T            open_mode,
	const DL_FS_OWNER_T                owner_id
);

/*
 * This function is called by application to close a file.
 */
extern DL_FS_RESULT_T DL_FsCloseFile(DL_FS_HANDLE_T file_handle);

/*
 * This function is called by application to read from a file.
 */
extern DL_FS_RESULT_T DL_FsReadFile(
	void *                             buffer,
	const DL_FS_SIZE_T                 element_size,
	const DL_FS_COUNT_T                count,
	DL_FS_HANDLE_T                     file_handle,
	DL_FS_COUNT_T *                    elements_read
);

/*
 * This function is called by application to write in a file.
 */
extern DL_FS_RESULT_T DL_FsWriteFile(
	void *                             buffer,
	const DL_FS_SIZE_T                 element_size,
	const DL_FS_COUNT_T                count,
	DL_FS_HANDLE_T                     file_handle,
	DL_FS_COUNT_T *                    elements_written
);

/*
 * This function is called by application to seek in a file.
 */
extern DL_FS_RESULT_T DL_FsFSeekFile(
	DL_FS_HANDLE_T                     file_handle,
	const DL_FS_SEEK_OFFSET_T          offset,
	const DL_FS_SEEK_FROM_T            seek_from
);

/*
 * This function is called by application to find out size of the file by file handle.
 */
extern DL_FS_SIZE_T DL_FsGetFileSize(DL_FS_HANDLE_T file_handle);

/*
 * This function is called by application to find out size of the file by file path.
 */
extern DL_FS_SIZE_T DL_FsSGetFileSize(const WCHAR *file_uri, const DL_FS_OWNER_T owner_id);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_DL_FILE_SYSTEM_H */
