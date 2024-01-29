#include "global.h"
#include <unistd.h>

//Freed after some time
xcb_keysym_t* keysyms;
xcb_get_keyboard_mapping_reply_t *kmapping;

//WM
void (**shortcut_lookup) (void);
size_t shortcut_lookup_offset;
size_t shortcut_lookup_l;
xcb_keycode_t normal_code;
MODE mode = MODE_NORMAL;

window_t *windows;
view_t view;

//XCB
xcb_connection_t* conn;
const xcb_setup_t* setup;
xcb_screen_t* screen;

void sh(char* cmd) {
  int pid = fork();
  if(pid == 0) {
    execl("/bin/sh", "sh", "-c", cmd, NULL);
  }
}
