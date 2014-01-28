#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdarg.h>
#include "version.h"

const char *version(void);
void error(const char *format, ...);

#endif

