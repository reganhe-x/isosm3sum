#include "libcheckisosm3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <popt.h>
#include <termios.h>

#include "sm3.h"

struct progressCBData {
    int verbose;
    int gauge;
    int gaugeat;
};

int user_bailing_out(void)
{
    struct timeval timev;
    fd_set rfds;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    timev.tv_sec = 0;
    timev.tv_usec = 0;

    if (select(1, &rfds, NULL, NULL, &timev) && getchar() == 27)
        return 1;

    return 0;
}

static int outputCB(void *const co, const long long offset, const long long total)
{
    struct progressCBData *const data = co;
    int gaugeval = -1;

    if (data->verbose) {
        printf("\rChecking: %05.1f%%", (100.0 * (double)offset) / (double)total);
        fflush(stdout);
    }

    if (data->gauge) {
        gaugeval = (int)((100.0 * (double)offset) / (double)total);
        if (gaugeval != data->gaugeat) {
            printf("%d\n", gaugeval);
            fflush(stdout);
            data->gaugeat = gaugeval;
        }
    }
    return user_bailing_out();
}

static int usage(void)
{
    fprintf(stderr, "Usage: checkisosm3 [--sm3sumonly] [--verbose] [--gauge] <isofilename>|<blockdevice>\n\n");
    return 1;
}

/* Process the result code and return the proper exit status value
 *
 * return 1 for failures, 0 for good checksum and 2 if aborted.
 */
int processExitStatus(const int rc)
{
    char *result;
    int exit_rc;

    switch (rc) {
        case ISOSM3SUM_CHECK_FAILED:
            result = "FAIL.\n\nIt is not recommended to use this media.";
            exit_rc = 1;
            break;
        case ISOSM3SUM_CHECK_ABORTED:
            result = "UNKNOWN.\n\nThe media check was aborted.";
            exit_rc = 2;
            break;
        case ISOSM3SUM_CHECK_NOT_FOUND:
            result = "NA.\n\nNo checksum information available, unable to verify media.";
            exit_rc = 1;
            break;
        case ISOSM3SUM_FILE_NOT_FOUND:
            result = "NA.\n\nFile not found.";
            exit_rc = 1;
            break;
        case ISOSM3SUM_CHECK_PASSED:
            result = "PASS.\n\nIt is OK to use this media.";
            exit_rc = 0;
            break;
        default:
            result = "checkisosm3 ERROR - bad return value";
            exit_rc = 1;
            break;
    }

    fprintf(stderr, "\nThe media check is complete, the result is: %s\n", result);

    return exit_rc;
}

int main(int argc, const char **argv)
{
    struct progressCBData data;
    memset(&data, 0, sizeof(data));
    data.verbose = 0;
    data.gauge = 0;

    int sm3only = 0;
    int help = 0;

    struct poptOption options[] = {{"sm3sumonly", 'o', POPT_ARG_NONE, &sm3only, 0},
                                   {"verbose", 'v', POPT_ARG_NONE, &data.verbose, 0},
                                   {"gauge", 'g', POPT_ARG_NONE, &data.gauge, 0},
                                   {"help", 'h', POPT_ARG_NONE, &help, 0},
                                   {0, 0, 0, 0, 0}};

    poptContext optCon = poptGetContext("checkisosm3", argc, argv, options, 0);

    int rc = poptGetNextOpt(optCon);
    if (rc < -1) {
        fprintf(stderr, "bad option %s: %s\n", poptBadOption(optCon, POPT_BADOPTION_NOALIAS), poptStrerror(rc));
        poptFreeContext(optCon);
        return 1;
    }

    if (help) {
        poptFreeContext(optCon);
        return usage();
    }

    const char **args = poptGetArgs(optCon);
    if (!args || !args[0] || !args[0][0]) {
        poptFreeContext(optCon);
        return usage();
    }

    if (sm3only | data.verbose) {
        rc = printSM3SUM(args[0]);
        if (rc < 0) {
            poptFreeContext(optCon);
            return processExitStatus(rc);
        }
    }

    if (sm3only) {
        poptFreeContext(optCon);
        return 0;
    }

    printf("Press [Esc] to abort check.\n");

    static struct termios oldt;
    struct termios newt;
    tcgetattr(0, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO | ECHONL | ISIG | IEXTEN);
    tcsetattr(0, TCSANOW, &newt);
    rc = mediaCheckFile(args[0], outputCB, &data);
    tcsetattr(0, TCSANOW, &oldt);

    if (data.verbose)
        printf("\n");

    poptFreeContext(optCon);
    return processExitStatus(rc);
}
