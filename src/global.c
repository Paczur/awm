#include "global.h"

//WM
MODE mode = MODE_NORMAL;
bool restart = false;

view_t view;

//XCB
xcb_connection_t* conn;
const xcb_setup_t* setup;
xcb_screen_t* screen;

//XLIB
Display *dpy;
XrmDatabase db;
XIM xim;
XIC xic;
