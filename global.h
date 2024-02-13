#ifndef H_GLOBAL
#define H_GLOBAL

#include <stddef.h>
#include <xcb/xcb.h>
#include "window.h"
#include "bar.h"
#include "system_config.h"

#define XK_LATIN1 //letters
#define XK_MISCELLANY //modifiers and special
#include <X11/keysymdef.h>
#include <X11/XF86keysym.h>
#include <X11/Xlib-xcb.h>
#include <X11/Xresource.h>

#define LENGTH(x) (sizeof((x))/sizeof((x)[0]))
#ifdef DEBUG
#undef DEBUG
#define DEBUG for(int _i=1; _i; _i=0, fflush(stdout))
#else
#define DEBUG if(false)
#endif

typedef unsigned char uchar;

typedef enum {
  MODE_NORMAL,
  MODE_INSERT
} MODE;

typedef enum KEY_T {
  KEY_ESC,
  KEY_RETURN,
  KEY_BACKSPACE,
  KEY_RIGHT,
  KEY_LEFT,
  KEY_LENGTH
} KEY_T;

typedef struct internal_shortcut_t {
  uint16_t mod_mask;
  void (*function) (void);
  struct internal_shortcut_t *next;
} internal_shortcut_t;

typedef struct view_t {
  size_t *spawn_order;
  size_t spawn_order_len;
  workspace_t workspaces[MAX_WORKSPACES];
  size_t focus;
  monitor_t *monitors;
  size_t monitor_count;
  bar_t *bars;
  bar_settings_t bar_settings;
  window_list_t *minimized;
  xcb_visualtype_t *visual_type;
} view_t;

int shout(char*, char*, size_t);
void sh(char*);
xcb_keycode_t keysym_to_keycode(xcb_keysym_t,
                                const xcb_get_keyboard_mapping_reply_t*);
//WM
extern internal_shortcut_t **shortcut_lookup;
extern size_t shortcut_lookup_offset;
extern size_t shortcut_lookup_l;
extern xcb_keycode_t normal_code;
extern MODE mode;
extern bool restart;
extern window_t *windows;
extern view_t view;
extern xcb_keycode_t *keys;

//XCB
extern xcb_connection_t *conn;
extern const xcb_setup_t *setup;
extern xcb_screen_t *screen;

//XLIB
extern Display *dpy;
extern XIM xim;
extern XIC xic;

#endif
