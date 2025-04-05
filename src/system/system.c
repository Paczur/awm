#include "system.h"

#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void system_run_bg(const char *cmd) {
  int pid = fork();
  if(pid == 0) {
    int pid2 = vfork();
    if(pid2 == 0) {
      execl("/bin/sh", "sh", "-c", cmd, NULL);
    } else {
      exit(0);
    }
  }
}

int system_run(const char *cmd) {
  int status = 0;
  int pid = fork();
  if(pid == 0) {
    execl("/bin/sh", "sh", "-c", cmd, NULL);
  } else {
    waitpid(pid, &status, 0);
  }
  return status;
}
