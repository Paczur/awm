#ifndef H_AWM_X_ATOM
#define H_AWM_X_ATOM

#include <xcb/xcb.h>

#define STANDARD_ATOMS                 \
  X(UTF8_STRING);                      \
  X(WM_NAME);                          \
  X(WM_PROTOCOLS);                     \
  X(WM_HINTS);                         \
  X(WM_NORMAL_HINTS);                  \
  X(WM_SIZE_HINTS);                    \
  X(WM_DELETE_WINDOW);                 \
  X(WM_CLASS);                         \
  X(WM_STATE);                         \
  X(WM_CHANGE_STATE);                  \
  X(WM_CLIENT_MACHINE);                \
  X(_NET_SUPPORTED);                   \
  X(_NET_CLIENT_LIST);                 \
  X(_NET_NUMBER_OF_DESKTOPS);          \
  X(_NET_CURRENT_DESKTOP);             \
  X(_NET_WM_DESKTOP);                  \
  X(_NET_ACTIVE_WINDOW);               \
  X(_NET_DESKTOP_NAMES);               \
  X(_NET_SUPPORTING_WM_CHECK);         \
  X(_NET_WM_NAME);                     \
  X(_NET_CLOSE_WINDOW);                \
  X(_NET_WM_ALLOWED_ACTIONS);          \
  X(_NET_WM_ACTION_MINIMIZE);          \
  X(_NET_WM_ACTION_CLOSE);             \
  X(_NET_FRAME_EXTENTS);               \
  X(_NET_REQUEST_FRAME_EXTENTS);       \
  X(_NET_WM_WINDOW_TYPE);              \
  X(_NET_WM_WINDOW_TYPE_SPLASH);       \
  X(_NET_WM_WINDOW_TYPE_UTILITY);      \
  X(_NET_WM_WINDOW_TYPE_NOTIFICATION); \
  X(_NET_WM_STATE);                    \
  X(_NET_WM_STATE_DEMANDS_ATTENTION);  \
  X(_NET_WM_STATE_HIDDEN);
#define AWM_ATOMS X(AWM_SHORTCUT_MODE)

#define X(x) extern xcb_atom_t x;
STANDARD_ATOMS;
AWM_ATOMS;
#undef X

#endif
