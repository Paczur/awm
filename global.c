#include "global.h"
#include <unistd.h>

//Freed after some time
xcb_keysym_t* keysyms;
xcb_get_keyboard_mapping_reply_t *kmapping;

//WM
void (**shortcut_lookup) (void);
size_t shortcut_lookup_offset;
size_t shortcut_lookup_l;
MODE mode = MODE_NORMAL;
xcb_keycode_t normal_code;

// TODO: MOVE TO LINKED LIST
window_t *windows;
size_t windows_length;
size_t windows_i = 0;

grid_cell_t *window_grid;
size_t grid_length;

monitor_t *monitors;
size_t monitors_length;

size_t current_window = SIZE_MAX;

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
