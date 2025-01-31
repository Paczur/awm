#include "log.h"

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../config.h"

#define TIMESTR_LENGTH 80

static int log_file = -1;

static void get_file(const char *dir) {
  char file[200];
  size_t dir_len = strlen(dir);
  time_t rawtime;
  struct tm *timeinfo;
  if(!strcmp(dir, "stdout")) {
    log_file = STDOUT_FILENO;
    return;
  }
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strcpy(file, dir);
  strftime(file + dir_len, 200 - dir_len, "/%F.log", timeinfo);
  log_file = open(file, O_WRONLY | O_CREAT | O_APPEND,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if(log_file == -1) log_file = STDOUT_FILENO;
}
static char level_char[] = {[LOG_LEVEL_FATAL] = 'F',
                            [LOG_LEVEL_ERROR] = 'E',
                            [LOG_LEVEL_WARNING] = 'W',
                            [LOG_LEVEL_INFO] = 'I'};
static void strtime(char *buff, size_t length) {
  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(buff, length, "%H:%M:%S", timeinfo);
}
static enum log_level log_level = LOG_LEVEL;

void log_internal(enum log_level level, awm_status status, const char *msg,
                  const char *component, const char *file, int line, ...) {
  va_list args;
  awm_assert(msg != NULL);
  awm_assert(file != NULL);
  char time[TIMESTR_LENGTH];
  if(log_level > level) return;
  strtime(time, TIMESTR_LENGTH);
  if(log_file < 0) get_file(LOG_DIR);
  awm_assert(log_file >= 0);
  dprintf(log_file, "[%s|%s|%c|%s|%s:%i] ", time, component, level_char[level],
          awm_status_str[status], file, line);
  va_start(args, line);
  vdprintf(log_file, msg, args);
  va_end(args);
  dprintf(log_file, "\n");
  fflush(stdout);
}

void log_level_set(enum log_level l) { log_level = l; }

enum log_level log_level_get(void) { return log_level; }
