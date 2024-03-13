#ifndef H_HINT_ATOM
#define H_HINT_ATOM

#include <xcb/xcb.h>

extern xcb_atom_t UTF8_STRING;
extern xcb_atom_t WM_NAME;
extern xcb_atom_t WM_PROTOCOLS;
extern xcb_atom_t WM_HINTS;
extern xcb_atom_t WM_DELETE_WINDOW;
extern xcb_atom_t WM_CLASS;
extern xcb_atom_t WM_STATE;
extern xcb_atom_t WM_CHANGE_STATE;
extern xcb_atom_t WM_CLIENT_MACHINE;
extern xcb_atom_t _NET_SUPPORTED;
extern xcb_atom_t _NET_CLIENT_LIST;
extern xcb_atom_t _NET_NUMBER_OF_DESKTOPS;
extern xcb_atom_t _NET_CURRENT_DESKTOP;
extern xcb_atom_t _NET_WM_DESKTOP;
extern xcb_atom_t _NET_ACTIVE_WINDOW;
extern xcb_atom_t _NET_DESKTOP_NAMES;
extern xcb_atom_t _NET_SUPPORTING_WM_CHECK;
extern xcb_atom_t _NET_WM_NAME;
extern xcb_atom_t _NET_CLOSE_WINDOW;
extern xcb_atom_t _NET_WM_ALLOWED_ACTIONS;
extern xcb_atom_t _NET_WM_ACTION_MINIMIZE;
extern xcb_atom_t _NET_WM_ACTION_CLOSE;
extern xcb_atom_t _NET_FRAME_EXTENTS;
extern xcb_atom_t _NET_REQUEST_FRAME_EXTENTS;
extern xcb_atom_t _NET_WM_WINDOW_TYPE;
extern xcb_atom_t _NET_WM_WINDOW_TYPE_SPLASH;
extern xcb_atom_t _NET_WM_STATE;
extern xcb_atom_t _NET_WM_STATE_DEMANDS_ATTENTION;
extern xcb_atom_t _NET_WM_STATE_HIDDEN;

void atom_init(xcb_connection_t*);
void atom_deinit(void);

#endif
