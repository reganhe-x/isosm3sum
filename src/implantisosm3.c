#include "libimplantisosm3.h"

#include <stdio.h>
#include <stdlib.h>
#include <popt.h>

#include "sm3.h"

static int usage(void)
{
    fprintf(stderr, "implantisosm3:         implantisosm3 [--force] [--supported-iso] <isofilename>\n");
    return 1;
}

int main(int argc, const char **argv)
{
    char *errstr;

    int forceit = 0;
    int supported = 0;
    int help = 0;

    struct poptOption options[] = {{"force", 'f', POPT_ARG_NONE, &forceit, 0},
                                   {"supported-iso", 'S', POPT_ARG_NONE, &supported, 0},
                                   {"help", 'h', POPT_ARG_NONE, &help, 0},
                                   {0, 0, 0, 0, 0}};

    poptContext optCon = poptGetContext("implantisosm3", argc, argv, options, 0);

    int rc;
    if ((rc = poptGetNextOpt(optCon)) < -1) {
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

    rc = implantISOFile(args[0], supported, forceit, 0, &errstr);
    if (rc) {
        fprintf(stderr, "ERROR: ");
        fprintf(stderr, errstr, args[0]);
        fprintf(stderr, "\n\n");
        rc = 1;
    }
    poptFreeContext(optCon);
    return rc;
}
