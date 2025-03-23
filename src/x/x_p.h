#ifndef H_AWM_X_PRIVATE
#define H_AWM_X_PRIVATE

#include <xcb/randr.h>
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xkb.h>

#define ATOMS                   \
  X(AWM_MODE)                   \
  X(AWM_VISIBLE_WORKSPACES)     \
  X(AWM_FOCUSED_MONITOR)        \
  X(AWM_FOCUSED_WINDOWS)        \
  X(AWM_MINIMIZED_WINDOWS)      \
  X(AWM_MINIMIZED_WINDOW_COUNT) \
  X(AWM_SIZE_OFFSETS)           \
  X(AWM_FULLSCREEN_WINDOWS)     \
  X(AWM_WORKSPACE_0)            \
  X(AWM_WORKSPACE_1)            \
  X(AWM_WORKSPACE_2)            \
  X(AWM_WORKSPACE_3)            \
  X(AWM_WORKSPACE_4)            \
  X(AWM_WORKSPACE_5)            \
  X(AWM_WORKSPACE_6)            \
  X(AWM_WORKSPACE_7)            \
  X(AWM_WORKSPACE_8)            \
  X(AWM_WORKSPACE_9)            \
  X(UTF8_STRING)                \
  X(WM_PROTOCOLS)               \
  X(WM_DELETE_WINDOW)           \
  X(WM_CLIENT_MACHINE)          \
  X(WM_CLASS)                   \
  X(_NET_ACTIVE_WINDOW)         \
  X(_NET_NUMBER_OF_DESKTOPS)    \
  X(_NET_CURRENT_DESKTOP)       \
  X(_NET_SUPPORTED)             \
  X(_NET_WM_NAME)               \
  X(_NET_SUPPORTING_WM_CHECK)   \
  X(_NET_WM_STATE)              \
  X(_NET_WM_STATE_FULLSCREEN)

#define WM_NAME XCB_ATOM_WM_NAME
#define WM_ICON_NAME XCB_ATOM_WM_ICON_NAME

#define X(x) extern xcb_atom_t x;
ATOMS
#undef X

extern xcb_visualtype_t *visual_type;
extern xcb_connection_t *conn;
extern const xcb_setup_t *setup;
extern xcb_screen_t *screen;
extern u8 xkb_event;
extern u8 randr_event;

u32 query_cardinal(xcb_atom_t atom, u32 def);
void send_cardinal(xcb_atom_t atom, u32 val);

void delete_window_property(u32 window, xcb_atom_t atom);

u32 query_window_cardinal_array(u32 window, xcb_atom_t atom, u32 *arr,
                                u32 length);
void send_window_cardinal_array(u32 window, xcb_atom_t atom, u32 *arr,
                                u32 length);
void append_window_cardinal_array(u32 window, xcb_atom_t atom, u32 val);

u32 query_window_atom_array(u32 window, xcb_atom_t atom, xcb_atom_t *arr,
                            u32 length);
void send_window_atom_array(u32 window, xcb_atom_t atom, xcb_atom_t *arr,
                            u32 length);
void append_window_atom_array(u32 window, xcb_atom_t atom, xcb_atom_t val);

u32 query_cardinal_array(xcb_atom_t atom, u32 *arr, u32 length);
void send_cardinal_array(xcb_atom_t atom, u32 *arr, u32 length);
void query_window_string(xcb_window_t window, xcb_atom_t atom, char *string,
                         u32 *string_len, u32 string_size);

void map_window(xcb_window_t window);
void unmap_window(xcb_window_t window);

void xkb_state_notify(xcb_xkb_state_notify_event_t *event);
u32 keycode_to_utf8(u8 keycode, char *buff, u32 size);

#endif
