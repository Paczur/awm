#ifndef H_SHARED_RECT
#define H_SHARED_RECT

typedef unsigned char uchar;

#include <stdint.h>

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

typedef enum WINDOW_STATE {
  WINDOW_INVALID = -3,
  WINDOW_WITHDRAWN = -2,
  WINDOW_ICONIC = -1,
  WINDOW_WORKSPACE_START = 0,
} WINDOW_STATE;

typedef struct list_t {
  struct list_t *next;
} list_t;

typedef struct double_list_t {
  struct double_list_t *next;
  struct double_list_t *prev;
} double_list_t;

typedef struct plist_t {
  struct plist_t *next;
  char *p;
} plist_t;

typedef struct rect_t {
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
} rect_t;

typedef enum {
  MODE_NORMAL,
  MODE_INSERT,
  MODE_INVALID
} MODE;

#define OUT_PREFIX(val) printf("\t\"" #val "\": ")
#define OUT_SUFFIX() puts(",")
#define OUT_RAW(val) \
  printf(\
         _Generic((val), \
                  bool:           (val)?"true":"false (%d)", \
                  char:           "%c", \
                  signed char:    "%d", \
                  short:          "%d", \
                  int:            "%d", \
                  long:           "%ld", \
                  unsigned char:  "%u", \
                  unsigned short: "%u", \
                  unsigned int:   "%u", \
                  unsigned long:  "%lu", \
                  char*:          "\"%s\"", \
                  const char*:    "\"%s\"", \
                  float:          "%f", \
                  double:         "%lf", \
                  default:        "%p"), (val));
#define OUT(val) \
  OUT_PREFIX(val); \
  OUT_RAW(val); \
  OUT_SUFFIX();

#define OUT_ARR_PREFIX(val) OUT_PREFIX(val); printf("[ ");
#define OUT_ARR_SUFFIX() printf(" ]"); OUT_SUFFIX()
#define OUT_ARR_NEXT(val) printf(", ");
#define OUT_ARR_GENERIC(val, count, out) \
  do { \
    OUT_ARR_PREFIX(val); \
    if(count > 0) { \
      for(size_t _i=0; _i<(count)-1; _i++) { \
        out((val)[_i]); \
        OUT_ARR_NEXT(val[_i]); \
      } \
      OUT_RAW(val[(count)-1]); \
    } \
    OUT_ARR_SUFFIX(); \
  } while(0)
#define OUT_ARR(val, count) OUT_ARR_GENERIC(val, count, OUT_RAW)

#define OUT_MODE(mode) \
  do { \
    OUT_PREFIX(mode); \
    puts((mode)==MODE_NORMAL?"normal":(mode==MODE_INSERT)?"insert":"invalid"); \
  } while(0)
#define OUT_MODE_ARR(mode, count) OUT_ARR_GENERIC(mode, count, OUT_MODE)

#define OUT_WINDOW_STATE(state) \
  do { \
    OUT_PREFIX(state); \
      if((state)==WINDOW_WITHDRAWN) { \
        puts("withdrawn"); \
      } else if((state)==WINDOW_ICONIC) { \
        puts("iconic"); \
      } else if((state)==WINDOW_INVALID) { \
        printf("invalid"); \
      } else { \
        printf("%d\n", state); \
      } \
  } while(0)
#define OUT_WINDOW_STATE_ARR(state, count) OUT_ARR_GENERIC(mode, count, OUT_MODE)

#define OUT_RECT(r) \
  do { \
    OUT(r->x); \
    OUT(r->y); \
    OUT(r->w); \
    OUT(r->h); \
  } while(0)
#define OUT_RECT_ARR(r, count) OUT_ARR_GENERIC(r, count, OUT_RECT)

#ifdef DEBUG
#undef DEBUG
#define DEBUG 1
#else
#define DEBUG 0
#endif

#ifdef TRACE
#undef TRACE
#define TRACE 1
#undef DEBUG
#define DEBUG 1
#else
#define TRACE 0
#endif

#define _STR(x) #x
#define STR(x) _STR(x)

#define _LOG(level, msg) \
  do { \
    if(level) { \
      printf("%s {\n", msg); \
      PRINT \
      puts("}"); \
    } \
  } while(0)

#define _LOGF(level) \
  do { \
    if(level) { \
      printf("%s {\n", __func__); \
      PRINT \
      puts("}"); \
    } \
  } while(0)

#define _LOGFE(level) printf("%s\n", __func__)

#define _LOGE(level, text) \
  if(level) puts(text)

#define LOG _LOG
#define LOGF _LOGF
#define LOGFE _LOGFE
#define LOGE _LOGE

#endif
