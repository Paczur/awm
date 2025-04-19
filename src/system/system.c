#include "system.h"

#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../types.h"
#include "system_x.h"

static void system_run_bg_raw(const char *cmd, const char *redir) {
  int status;
  int pid = fork();
  if(pid == 0) {
    int pid2 = vfork();
    if(pid2 == 0) {
      execl("/bin/sh", "bash", "-c", cmd, redir, NULL);
    } else {
      exit(0);
    }
  } else {
    waitpid(pid, &status, 0);
  }
}

void system_run_bg(const char *cmd) {
  system_run_bg_raw(cmd, " >/dev/null 2>&1");
}

int system_run_thread(const char *cmd) {
  int status = 0;
  int pid = vfork();
  if(pid == 0) {
    execl("/bin/sh", "bash", "-c", cmd, NULL);
  } else {
    waitpid(pid, &status, 0);
  }
  return status;
}

int system_run(const char *cmd) {
  int status = 0;
  int pid = vfork();
  if(pid == 0) {
    execl("/bin/sh", "bash", "-c", cmd, NULL);
  } else {
    send_changes();
    waitpid(pid, &status, 0);
  }
  return status;
}
