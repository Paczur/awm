#include "system.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>


//XCB
xcb_visualtype_t *visual_type;
xcb_connection_t* conn;
const xcb_setup_t* setup;
xcb_screen_t* screen;

//XLIB
XIM xim;
XIC xic;
Display *dpy;

int shout(char *cmd, char *out, size_t len) {
  FILE *f;
  int pid;
  int status;
  int fd[2];
  if(!pipe(fd)) {
    pid = fork();
    if(pid == 0) {
      close(fd[0]);
      dup2(fd[1], STDOUT_FILENO);
      close(fd[1]);
      execl("/bin/sh", "sh", "-c", cmd, NULL);
    } else {
      close(fd[1]);
      f = fdopen(fd[0], "r");
      if(f && !fgets(out, len, f)) {
        out[strcspn(out, "\n")] = 0;
      }
      waitpid(pid, &status, 0);
      if(WIFEXITED(status)) {
        status = WEXITSTATUS(status);
      }
    }
  }
  return status;
}

void sh(char* cmd) {
  int pid = vfork();
  if(pid == 0) {
    execl("/bin/sh", "sh", "-c", cmd, NULL);
  }
}
