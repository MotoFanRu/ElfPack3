#ifndef P2K_EP3_COMPONENT_H
#define P2K_EP3_COMPONENT_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PATH_MAX_COMPONENT             (32)

/*
 * EXL, 15-Nov-2026:
 *
 * Функция поиска внутренних компонентов проекта по файловой системе телефона.
 * Диски задаются с помощью предопределённых на этапе компиляции макросов:
 *     NONE (нет определений) - Плоская файловая система.
 *     FTR_DISKS_A            - Только диск /a/.
 *     FTR_DISKS_AB           - Диски /b/, /a/.
 *     FTR_DISKS_ABC          - Диски /b/, /c/, /a/.
 *     FTR_DISKS_ALL          - Диски /b/, /c/, /e/, /g/, /a/.
 * Приоритет и порядок поиска на дисках задаётся в массиве `disks[]`.
 *
 * Функция принимает аргументом имя компонента `file_name` и указатель
 * на строку `out_path` в которую будет скопирован путь в случае успеха.
 * В случае если файл не будет найден в `out_path` будет `NULL`.
 *
 * Функция возвращает:
 *     RESULT_OK
 *     RESULT_FAIL_WRONG_ARG
 *     RESULT_FAIL_FILE_NOT_FOUND
 */
extern STATUS EP3_Find_Component(const char *file_name, WCHAR *out_path);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_COMPONENT_H */
