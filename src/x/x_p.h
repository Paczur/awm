#ifndef H_AWM_X_PRIVATE
#define H_AWM_X_PRIVATE

#include <xcb/xcb.h>

extern xcb_atom_t AWM_MODE;
extern xcb_visualtype_t *visual_type;
extern xcb_connection_t *conn;
extern const xcb_setup_t *setup;
extern xcb_screen_t *screen;

#endif
