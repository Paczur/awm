#include "global.h"
#include <unistd.h>
#include <sys/wait.h>

//WM
shortcuts_t shortcuts;
MODE mode = MODE_NORMAL;
bool restart = false;

window_t *windows;
view_t view;

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

bool handle_shortcut(const shortcut_map_t *map, xcb_keycode_t keycode,
                     uint16_t state) {
  shortcut_t *sh;
  size_t lookup;

  lookup = keycode - map->offset;
  if(lookup >= map->length)
    return false;
  sh = map->values[lookup];
  while(sh != NULL) {
    if(state == sh->mod_mask) {
      sh->function();
      return true;
    } else {
      sh = sh->next;
    }
  }
  return false;
}

void free_shortcut(shortcut_map_t *map) {
  shortcut_t *sh;
  shortcut_t *t;
  for(size_t i=0; i<map->length; i++) {
    sh = map->values[i];
    while(sh != NULL) {
      t = sh;
      sh = sh->next;
      free(t);
    }
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
