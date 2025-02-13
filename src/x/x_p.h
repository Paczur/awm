#ifndef H_AWM_X_PRIVATE
#define H_AWM_X_PRIVATE

#include <xcb/randr.h>
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>

#define ATOMS               \
  X(AWM_MODE)               \
  X(AWM_VISIBLE_WORKSPACES) \
  X(AWM_WORKSPACE_0)        \
  X(AWM_WORKSPACE_1)        \
  X(AWM_WORKSPACE_2)        \
  X(AWM_WORKSPACE_3)        \
  X(AWM_WORKSPACE_4)        \
  X(AWM_WORKSPACE_5)        \
  X(AWM_WORKSPACE_6)        \
  X(AWM_WORKSPACE_7)        \
  X(AWM_WORKSPACE_8)        \
  X(AWM_WORKSPACE_9)        \
  X(WM_PROTOCOLS)           \
  X(WM_DELETE_WINDOW)       \
  X(_NET_ACTIVE_WINDOW)

#define X(x) extern xcb_atom_t x;
ATOMS
#undef X

extern xcb_visualtype_t *visual_type;
extern xcb_connection_t *conn;
extern const xcb_setup_t *setup;
extern xcb_screen_t *screen;

u32 query_cardinal(xcb_atom_t atom, u32 def);
void send_cardinal(xcb_atom_t atom, u32 val);
void query_cardinal_array(xcb_atom_t atom, u32 *arr, u32 length);
void send_cardinal_array(xcb_atom_t atom, u32 *arr, u32 length);

#endif
