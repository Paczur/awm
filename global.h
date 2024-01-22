#ifndef H_GLOBAL
#define H_GLOBAL

#include <stddef.h>
#include <xcb/xcb.h>
#include "window.h"

#define LENGTH(x) (sizeof(x)/sizeof(x[0]))

typedef enum {
  MODE_NORMAL,
  MODE_INSERT
} MODE;

void sh(char*);

//Freed after some time
extern xcb_keysym_t* keysyms;
extern xcb_get_keyboard_mapping_reply_t *kmapping;

//WM
extern void (**shortcut_lookup) (void);
extern size_t shortcut_lookup_offset;
extern size_t shortcut_lookup_l;
extern MODE mode;
extern xcb_keycode_t normal_code;

extern window_t *windows;
extern size_t windows_length;
extern size_t windows_i;

extern monitor_t *monitors;
extern size_t monitors_length;

extern grid_cell_t *window_grid;
extern size_t grid_length;

extern size_t current_window;

//XCB
extern xcb_connection_t* conn;
extern const xcb_setup_t* setup;
extern xcb_screen_t* screen;

#endif
