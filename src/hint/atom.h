#ifndef H_HINT_ATOM
#define H_HINT_ATOM

#include <xcb/xcb.h>

extern xcb_atom_t WM_PROTOCOLS;
extern xcb_atom_t WM_DELETE_WINDOW;
extern xcb_atom_t WM_CLASS;
extern xcb_atom_t WM_STATE;
extern xcb_atom_t WM_CHANGE_STATE;
extern xcb_atom_t WM_CLIENT_MACHINE;
extern xcb_atom_t _NET_SUPPORTED;
extern xcb_atom_t _NET_CLIENT_LIST;

void atom_init(xcb_connection_t*);
void atom_deinit(void);

#endif
