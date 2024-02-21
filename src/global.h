#ifndef H_GLOBAL
#define H_GLOBAL

#include <stddef.h>
#include <xcb/xcb.h>
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

//WM
extern MODE mode;
extern bool restart;

extern XIM xim;
extern XIC xic;

#endif
