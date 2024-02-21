#ifndef H_SYSTEM
#define H_SYSTEM

#include <xcb/xcb.h>
#include <X11/Xlib-xcb.h>
#include <stddef.h>


//XCB
extern xcb_visualtype_t *visual_type;
extern xcb_connection_t *conn;
extern const xcb_setup_t *setup;
extern xcb_screen_t *screen;

//XLIB
extern XIM xim;
extern XIC xic;
extern Display *dpy;

int shout(char*, char*, size_t);
void sh(char*);

#endif
