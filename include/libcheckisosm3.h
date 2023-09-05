#ifndef __LIBCHECKISOSM3_H__
#define __LIBCHECKISOSM3_H__

#ifdef __cplusplus
extern "C" {
#endif

enum isosm3sum_status {
    ISOSM3SUM_FILE_NOT_FOUND = -2,
    ISOSM3SUM_CHECK_NOT_FOUND = -1,
    ISOSM3SUM_CHECK_FAILED = 0,
    ISOSM3SUM_CHECK_PASSED = 1,
    ISOSM3SUM_CHECK_ABORTED = 2
};

/* For non-zero return value, check is aborted. */
typedef int (*checkCallback)(void *, long long offset, long long total);

int mediaCheckFile(const char *file, checkCallback cb, void *cbdata);
int mediaCheckFD(int isofd, checkCallback cb, void *cbdata);
int printSM3SUM(const char *file);

#ifdef __cplusplus
}
#endif

#endif
