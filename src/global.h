#ifndef H_GLOBAL
#define H_GLOBAL

#include <stddef.h>
#include <xcb/xcb.h>
#include "bar.h"
#include "protocol.h"
#include "system_config.h"
#include <stdbool.h>

#define XK_LATIN1 //letters
#define XK_MISCELLANY //modifiers and special
#include <X11/keysymdef.h>
#include <X11/XF86keysym.h>
#include <X11/Xlib-xcb.h>
#include <X11/Xresource.h>

#define LENGTH(x) (sizeof((x))/sizeof((x)[0]))

typedef unsigned char uchar;

typedef enum {
  MODE_NORMAL,
  MODE_INSERT
} MODE;

typedef struct view_t {
  bar_t *bars;
  bar_settings_t bar_settings;
  xcb_visualtype_t *visual_type;
} view_t;

//WM
extern MODE mode;
extern bool restart;
extern view_t view;

extern XIM xim;
extern XIC xic;

#endif
