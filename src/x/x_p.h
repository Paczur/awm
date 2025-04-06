#ifndef H_AWM_X_PRIVATE
#define H_AWM_X_PRIVATE

#include <xcb/randr.h>
#include <xcb/xcb.h>
#include <xcb/xkb.h>

#define ATOMS                     \
  X(AWM_MODE)                     \
  X(AWM_COLORSCHEME)              \
  X(AWM_VISIBLE_WORKSPACES)       \
  X(AWM_FOCUSED_MONITOR)          \
  X(AWM_FOCUSED_WINDOWS)          \
  X(AWM_MINIMIZED_WINDOWS)        \
  X(AWM_MINIMIZED_WINDOW_COUNT)   \
  X(AWM_SIZE_OFFSETS)             \
  X(AWM_FULLSCREEN_WINDOWS)       \
  X(AWM_URGENT_WORKSPACE_WINDOWS) \
  X(AWM_URGENT_MINIMIZED_WINDOWS) \
  X(AWM_WORKSPACES)               \
  X(AWM_FLOATING_WORKSPACES)      \
  X(UTF8_STRING)                  \
  X(WM_PROTOCOLS)                 \
  X(WM_DELETE_WINDOW)             \
  X(WM_CLIENT_MACHINE)            \
  X(WM_CLASS)                     \
  X(WM_HINTS)                     \
  X(WM_NORMAL_HINTS)              \
  X(WM_SIZE_HINTS)                \
  X(_NET_ACTIVE_WINDOW)           \
  X(_NET_NUMBER_OF_DESKTOPS)      \
  X(_NET_CURRENT_DESKTOP)         \
  X(_NET_SUPPORTED)               \
  X(_NET_WM_NAME)                 \
  X(_NET_SUPPORTING_WM_CHECK)     \
  X(_NET_WM_STATE)                \
  X(_NET_WM_STATE_FULLSCREEN)     \
  X(_NET_WM_STATE_HIDDEN)         \
  X(_NET_WM_STATE_FOCUSED)        \
  X(_NET_WM_ALLOWED_ACTIONS)      \
  X(_NET_WM_ACTION_FULLSCREEN)    \
  X(_NET_WM_ACTION_MINIMIZE)      \
  X(_NET_WM_ACTION_CLOSE)         \
  X(_NET_CLOSE_WINDOW)            \
  X(_NET_WM_SYNC_REQUEST)         \
  X(_NET_WM_SYNC_REQUEST_COUNTER)

#define WM_NAME XCB_ATOM_WM_NAME
#define WM_ICON_NAME XCB_ATOM_WM_ICON_NAME

#define X(x) extern xcb_atom_t x;
ATOMS
#undef X

struct wm_hints {
  struct {
    u8 input : 1;
    u8 initial_state : 1;
    u8 icon_pixmap : 1;
    u8 icon_window : 1;
    u8 icon_position : 1;
    u8 icon_mask : 1;
    u8 window_group : 1;
    u8 obsolete_message : 1;
    u8 urgency : 1;
  } flags;
  u32 input;
  u32 initial_state;
  u32 icon_pixmap;
  u32 icon_window;
  i32 icon_x;
  i32 icon_y;
  u32 icon_mask;
  u32 window_group;
};

struct wm_size_hints {
  struct {
    u8 user_position : 1;
    u8 user_size : 1;
    u8 program_position : 1;
    u8 program_size : 1;
    u8 program_min_size : 1;
    u8 program_max_size : 1;
    u8 program_resize_inc : 1;
    u8 program_aspect : 1;
    u8 program_base_size : 1;
    u8 program_win_gravity : 1;
  } flags;
  u32 pad[4];
  i32 min_width;
  i32 min_height;
  i32 max_width;
  i32 max_height;
  i32 width_inc;
  i32 height_inc;
  i32 min_aspect[2];
  i32 max_aspect[2];
  i32 base_width;
  i32 base_height;
  i32 win_gravity;
};

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

struct wm_hints query_window_hints(u32 window);
struct wm_size_hints query_window_size_hints(u32 window);

void map_window(xcb_window_t window);
void unmap_window(xcb_window_t window);

void xkb_state_notify(xcb_xkb_state_notify_event_t *event);
u32 keycode_to_utf8(u8 keycode, char *buff, u32 size);

void send_changes(void);

#endif
