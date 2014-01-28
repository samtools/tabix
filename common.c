#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <htslib/hts.h>
#include "common.h"

void error(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    exit(-1);
}

char version_string[250];

const char *version(void)
{
    snprintf(version_string,250,"%s+htslib-%s", VERSION,hts_version());
    return version_string;
}

