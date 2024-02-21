#ifndef H_SHARED_PROTOCOL
#define H_SHARED_PROTOCOL

#include <xcb/xcb.h>
#include <X11/Xlib-xcb.h>

extern xcb_visualtype_t *visual_type;

//XCB
extern xcb_connection_t *conn;
extern const xcb_setup_t *setup;
extern xcb_screen_t *screen;

//XLIB
extern Display *dpy;

#endif
