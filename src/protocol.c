#include "protocol.h"

xcb_visualtype_t *visual_type;

//XCB
xcb_connection_t* conn;
const xcb_setup_t* setup;
xcb_screen_t* screen;

//XLIB
Display *dpy;
