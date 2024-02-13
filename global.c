#include "global.h"
#include <unistd.h>
#include <sys/wait.h>

//WM
internal_shortcut_t **shortcut_lookup;
size_t shortcut_lookup_offset;
size_t shortcut_lookup_l;
xcb_keycode_t normal_code;
MODE mode = MODE_NORMAL;
bool restart = false;

window_t *windows;
view_t view;
xcb_keycode_t *keys;

//XCB
xcb_connection_t* conn;
const xcb_setup_t* setup;
xcb_screen_t* screen;

//XLIB
Display *dpy;
XrmDatabase db;
XIM xim;
XIC xic;

int shout(char *cmd, char *out, size_t len) {
  FILE *f;
  int pid;
  int status;
  int fd[2];
  pipe(fd);
  pid = fork();
  if(pid == 0) {
    close(fd[0]);
    dup2(fd[1], STDOUT_FILENO);
    close(fd[1]);
    execl("/bin/sh", "sh", "-c", cmd, NULL);
  } else {
    close(fd[1]);
    f = fdopen(fd[0], "r");
    if(f) {
      fgets(out, len, f);
      out[strcspn(out, "\n")] = 0;
    }
    waitpid(pid, &status, 0);
    if(WIFEXITED(status)) {
      status = WEXITSTATUS(status);
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

xcb_keycode_t keysym_to_keycode(xcb_keysym_t keysym,
                                const xcb_get_keyboard_mapping_reply_t *kmapping) {
  xcb_keysym_t* keysyms = xcb_get_keyboard_mapping_keysyms(kmapping);
  for(int i=setup->min_keycode; i<setup->max_keycode; i++) {
    if(keysyms[(i-setup->min_keycode) * kmapping->keysyms_per_keycode] == keysym)
      return i;
  }
  return -1;
}
