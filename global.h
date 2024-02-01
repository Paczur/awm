#ifndef H_GLOBAL
#define H_GLOBAL

#include <stddef.h>
#include <xcb/xcb.h>
#include "window.h"
#include "bar.h"

#define LENGTH(x) (sizeof(x)/sizeof(x[0]))
#ifdef DEBUG
#undef DEBUG
#define DEBUG for(int _i=1; _i; _i=0, fflush(stdout))
#else
#define DEBUG if(false)
#endif

typedef enum {
  MODE_NORMAL,
  MODE_INSERT
} MODE;

typedef struct view_t {
  workspace_t workspaces[10];
  size_t focus;
  monitor_t *monitors;
  size_t monitor_count;
  bar_t *bars;
  uint32_t bar_height;
  uint32_t bar_color;
  char *bar_font;
  xcb_visualtype_t *visual_type;
} view_t;

void sh(char*);

//Freed after some time
extern xcb_keysym_t* keysyms;
extern xcb_get_keyboard_mapping_reply_t *kmapping;

//WM
extern void (**shortcut_lookup) (void);
extern size_t shortcut_lookup_offset;
extern size_t shortcut_lookup_l;
extern xcb_keycode_t normal_code;
extern MODE mode;

extern window_t *windows;
extern view_t view;

//XCB
extern xcb_connection_t *conn;
extern const xcb_setup_t *setup;
extern xcb_screen_t *screen;

#endif
