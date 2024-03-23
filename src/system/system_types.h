#ifndef H_SYSTEM_TYPES
#define H_SYSTEM_TYPES

#include "../types.h"

#ifdef SYSTEM_DEBUG
#undef SYSTEM_DEBUG
#define SYSTEM_DEBUG 1
#else
#define SYSTEM_DEBUG 0
#endif

#ifdef SYSTEM_TRACE
#undef SYSTEM_TRACE
#define SYSTEM_TRACE 1
#undef SYSTEM_DEBUG
#define SYSTEM_DEBUG 1
#else
#define SYSTEM_TRACE 0
#endif

#endif
