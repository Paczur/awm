#define CONFIG_NORMAL_SHORTCUT XK_Super_L
#define CONFIG_SHORTCUTS { \
  {MOD_NONE,  XK_1,      focus_window_0},         \
  {MOD_NONE,  XK_2,      focus_window_1},         \
  {MOD_NONE,  XK_3,      focus_window_2},         \
  {MOD_NONE,  XK_4,      focus_window_3},         \
  {MOD_NONE,  XK_5,      focus_window_4},         \
  {MOD_NONE,  XK_6,      focus_window_5},         \
  {MOD_NONE,  XK_7,      focus_window_6},         \
  {MOD_NONE,  XK_8,      focus_window_7},         \
  {MOD_NONE,  XK_9,      focus_window_8},         \
  {MOD_NONE,  XK_0,      focus_window_9},         \
  {MOD_SHIFT, XK_1,      swap_window_0},          \
  {MOD_SHIFT, XK_2,      swap_window_1},          \
  {MOD_SHIFT, XK_3,      swap_window_2},          \
  {MOD_SHIFT, XK_4,      swap_window_3},          \
  {MOD_SHIFT, XK_5,      swap_window_4},          \
  {MOD_SHIFT, XK_6,      swap_window_5},          \
  {MOD_SHIFT, XK_7,      swap_window_6},          \
  {MOD_SHIFT, XK_8,      swap_window_7},          \
  {MOD_SHIFT, XK_9,      swap_window_8},          \
  {MOD_SHIFT, XK_0,      swap_window_9},          \
  {MOD_CTRL,  XK_1,      workspace_0},            \
  {MOD_CTRL,  XK_2,      workspace_1},            \
  {MOD_CTRL,  XK_3,      workspace_2},            \
  {MOD_CTRL,  XK_4,      workspace_3},            \
  {MOD_CTRL,  XK_5,      workspace_4},            \
  {MOD_CTRL,  XK_6,      workspace_5},            \
  {MOD_CTRL,  XK_7,      workspace_6},            \
  {MOD_CTRL,  XK_8,      workspace_7},            \
  {MOD_CTRL,  XK_9,      workspace_8},            \
  {MOD_CTRL,  XK_0,      workspace_9},            \
  {MOD_NONE,  XK_h,      focus_window_left},      \
  {MOD_NONE,  XK_j,      focus_window_down},      \
  {MOD_NONE,  XK_k,      focus_window_up},        \
  {MOD_NONE,  XK_l,      focus_window_right},     \
  {MOD_SHIFT, XK_h,      swap_window_left},       \
  {MOD_SHIFT, XK_j,      swap_window_down},       \
  {MOD_SHIFT, XK_k,      swap_window_up},         \
  {MOD_SHIFT, XK_l,      swap_window_right},      \
  {MOD_NONE,  XK_q,      destroy_current_window}, \
  {MOD_NONE,  XK_Escape, insert_mode},            \
  {MOD_NONE,  XK_i,      insert_mode},            \
  {MOD_NONE,  XK_Return, terminal},               \
  {MOD_NONE,  XK_Left,   shrink_width},           \
  {MOD_NONE,  XK_Up,     shrink_height},          \
  {MOD_NONE,  XK_Right,  enlarge_width},          \
  {MOD_NONE,  XK_Down,   enlarge_height},         \
  {MOD_NONE,  XK_f,      librewolf}               \
}

#define CONFIG_TERMINAL_CMD "mlterm"
#define CONFIG_RESIZE_STEP 10

#define CONFIG_SPAWN_ORDER { 0, 4, 6, 1, 7, 3, 5, 2 }
#define CONFIG_GAPS 15

#define CONFIG_BAR_HEIGHT 32
#define CONFIG_BAR_FONT "CodeNewRoman Nerd Font 14"
#define CONFIG_BAR_COMPONENT_PADDING 8
#define CONFIG_BAR_COMPONENT_SEPARATOR 2
#define CONFIG_BAR_MODE_MIN_WIDTH 32
#define CONFIG_BAR_WORKSPACE_MIN_WIDTH 32

#define CONFIG_BAR_BACKGROUND "111111"
#define CONFIG_BAR_MODE_INSERT_BACKGROUND "111111"
#define CONFIG_BAR_MODE_INSERT_FOREGROUND "333333"
#define CONFIG_BAR_MODE_NORMAL_BACKGROUND "111111"
#define CONFIG_BAR_MODE_NORMAL_FOREGROUND "FFFFFF"

#define CONFIG_BAR_WORKSPACE_FOCUSED_BACKGROUND "111111"
#define CONFIG_BAR_WORKSPACE_FOCUSED_FOREGROUND "FFFFFF"
#define CONFIG_BAR_WORKSPACE_UNFOCUSED_BACKGROUND "111111"
#define CONFIG_BAR_WORKSPACE_UNFOCUSED_FOREGROUND "333333"
