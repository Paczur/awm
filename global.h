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

typedef struct shortcut_t {
  uint16_t mod_mask;
  void (*function) (void);
  struct shortcut_t *next;
} shortcut_t;

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

typedef struct shortcut_map_t {
  shortcut_t **values; //Can be NULL and are lists internally
  size_t offset;
  size_t length;
} shortcut_map_t;

typedef struct shortcuts_t {
  shortcut_map_t insert_mode;
  shortcut_map_t normal_mode;
  shortcut_map_t launcher;
} shortcuts_t;

int shout(char*, char*, size_t);
void sh(char*);
bool handle_shortcut(const shortcut_map_t*, xcb_keycode_t, uint16_t);
void free_shortcut(shortcut_map_t*);
xcb_keycode_t keysym_to_keycode(xcb_keysym_t,
                                const xcb_get_keyboard_mapping_reply_t*);
//WM
extern shortcuts_t shortcuts;
extern MODE mode;
extern bool restart;
extern window_t *windows;
extern view_t view;

//XCB
extern xcb_connection_t *conn;
extern const xcb_setup_t *setup;
extern xcb_screen_t *screen;

//XLIB
extern Display *dpy;
extern XIM xim;
extern XIC xic;

#endif
