#ifndef H_AWM_LOG
#define H_AWM_LOG

#include "../types.h"

enum log_level {
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARNING,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_FATAL,
};

void log_internal(enum log_level, awm_status, const char *msg,
                  const char *component, const char *file, int line, ...);

#define log_status(level, status, msg, ...)                                   \
  log_internal(level, status, msg, awm_current_component, __FILE__, __LINE__, \
               ##__VA_ARGS__)
#define log(level, msg, ...)                                               \
  log_internal(level, AWM_STATUS_OK, msg, awm_current_component, __FILE__, \
               __LINE__, ##__VA_ARGS__)

void log_level_set(enum log_level);
enum log_level log_level_get(void);

#endif
