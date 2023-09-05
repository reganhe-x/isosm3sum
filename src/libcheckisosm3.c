#include "libcheckisosm3.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "sm3.h"
#include "utilities.h"

static void
clear_appdata(unsigned char *const buffer, const size_t size, const off_t appdata_offset, const off_t offset)
{
    static const ssize_t buffer_start = 0;
    const ssize_t difference = appdata_offset - offset;
    if (-APPDATA_SIZE <= difference && difference <= (ssize_t)size) {
        const size_t clear_start = (size_t)MAX(buffer_start, difference);
        const size_t clear_len = MIN(size, (size_t)(difference + APPDATA_SIZE)) - clear_start;
        memset(buffer + clear_start, ' ', clear_len);
    }
}

static enum isosm3sum_status checksm3sum(int isofd, checkCallback cb, void *cbdata)
{
    struct volume_info *const info = parsepvd(isofd);
    if (info == NULL)
        return ISOSM3SUM_CHECK_NOT_FOUND;

    const off_t total_size = info->isosize - info->skipsectors * SECTOR_SIZE;
    const off_t fragment_size = total_size / (info->fragmentcount + 1);
    if (cb)
        cb(cbdata, 0LL, (long long)total_size);

    /* Rewind, compute sm3sum. */
    lseek(isofd, 0LL, SEEK_SET);

    SM3_CTX hashctx;
    SM3_Init(&hashctx);

    const size_t buffer_size = NUM_SYSTEM_SECTORS * SECTOR_SIZE;
    unsigned char *buffer;
    buffer = aligned_alloc((size_t)getpagesize(), buffer_size * sizeof(*buffer));

    size_t previous_fragment = 0UL;
    off_t offset = 0LL;
    while (offset < total_size) {
        const size_t nbyte = MIN((size_t)(total_size - offset), MIN(fragment_size, buffer_size));

        ssize_t nread = read(isofd, buffer, nbyte);
        if (nread <= 0L)
            break;

        /**
         * Originally was added in 2005 because the kernel was returning the
         * size from where it started up to the end of the block it pre-fetched
         * from a cd drive.
         */
        if (nread > nbyte) {
            nread = nbyte;
            lseek(isofd, offset + nread, SEEK_SET);
        }
        /* Make sure appdata which contains the sm3sum is cleared. */
        clear_appdata(buffer, nread, info->offset + APPDATA_OFFSET, offset);

        SM3_Update(&hashctx, buffer, (unsigned int)nread);
        if (info->fragmentcount) {
            const size_t current_fragment = offset / fragment_size;
            const size_t fragmentsize = FRAGMENT_SUM_SIZE / info->fragmentcount;
            /* If we're onto the next fragment, calculate the previous sum and check. */
            if (current_fragment != previous_fragment && current_fragment < info->fragmentcount) {
                if (!validate_fragment(&hashctx, current_fragment, fragmentsize, info->fragmentsums, NULL)) {
                    /* Exit immediately if current fragment sum is incorrect */
                    free(info);
                    free(buffer);
                    return ISOSM3SUM_CHECK_FAILED;
                }
                previous_fragment = current_fragment;
            }
        }
        offset += nread;
        if (cb)
            if (cb(cbdata, (long long)offset, (long long)total_size)) {
                free(info);
                free(buffer);
                return ISOSM3SUM_CHECK_ABORTED;
            }
    }
    free(buffer);

    if (cb)
        cb(cbdata, (long long)info->isosize, (long long)total_size);

    char hashsum[HASH_SIZE + 1];
    sm3sum(hashsum, &hashctx);

    int failed = strcmp(info->hashsum, hashsum);
    free(info);
    return failed ? ISOSM3SUM_CHECK_FAILED : ISOSM3SUM_CHECK_PASSED;
}

int mediaCheckFile(const char *file, checkCallback cb, void *cbdata)
{
    int isofd = open(file, O_RDONLY | O_BINARY);
    if (isofd < 0) {
        return ISOSM3SUM_FILE_NOT_FOUND;
    }
    int rc = checksm3sum(isofd, cb, cbdata);
    close(isofd);
    return rc;
}

int mediaCheckFD(int isofd, checkCallback cb, void *cbdata)
{
    return checksm3sum(isofd, cb, cbdata);
}

int printSM3SUM(const char *file)
{
    int isofd = open(file, O_RDONLY | O_BINARY);
    if (isofd < 0) {
        return ISOSM3SUM_FILE_NOT_FOUND;
    }
    struct volume_info *const info = parsepvd(isofd);
    close(isofd);
    if (info == NULL) {
        return ISOSM3SUM_CHECK_NOT_FOUND;
    }

    printf("%s:   %s\n", file, info->hashsum);
    if (strlen(info->fragmentsums) > 0 && info->fragmentcount > 0) {
        printf("Fragment sums: %s\n", info->fragmentsums);
        printf("Fragment count: %zu\n", info->fragmentcount);
        printf("Supported ISO: %s\n", info->supported ? "yes" : "no");
    }
    free(info);
    return 0;
}
