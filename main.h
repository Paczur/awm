#ifndef H_MAIN
#define H_MAIN

#include <xcb/xcb.h>
#include <X11/keysymdef.h>
#include <X11/XF86keysym.h>

xcb_keycode_t keysym_to_keycode(xcb_keysym_t);
xcb_keysym_t keycode_to_keysym(xcb_keycode_t);

#endif
