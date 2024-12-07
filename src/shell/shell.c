#include "shell.h"

#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../log/log.h"

void shell_run(const char *cmd) {
  int status;
  log(LOG_LEVEL_INFO, "Starting program: %s", cmd);
  int pid = fork();
  if(pid == 0) {
    int pid2 = vfork();
    if(pid2 == 0) {
      execl("/bin/sh", "sh", "-c", cmd, NULL);
    } else {
      exit(0);
    }
  } else {
    waitpid(pid, &status, 0);
  }
}
