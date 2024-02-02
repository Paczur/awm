xcb_keysym_t normal_shortcut = XK_Super_L;

shortcut_t shortcuts[] = {
  {MOD_NONE,  XK_1,      focus_window_0},
  {MOD_NONE,  XK_2,      focus_window_1},
  {MOD_NONE,  XK_3,      focus_window_2},
  {MOD_NONE,  XK_4,      focus_window_3},
  {MOD_NONE,  XK_5,      focus_window_4},
  {MOD_NONE,  XK_6,      focus_window_5},
  {MOD_NONE,  XK_7,      focus_window_6},
  {MOD_NONE,  XK_8,      focus_window_7},
  {MOD_NONE,  XK_9,      focus_window_8},
  {MOD_NONE,  XK_0,      focus_window_9},
  {MOD_SHIFT, XK_1,      swap_window_0},
  {MOD_SHIFT, XK_2,      swap_window_1},
  {MOD_SHIFT, XK_3,      swap_window_2},
  {MOD_SHIFT, XK_4,      swap_window_3},
  {MOD_SHIFT, XK_5,      swap_window_4},
  {MOD_SHIFT, XK_6,      swap_window_5},
  {MOD_SHIFT, XK_7,      swap_window_6},
  {MOD_SHIFT, XK_8,      swap_window_7},
  {MOD_SHIFT, XK_9,      swap_window_8},
  {MOD_SHIFT, XK_0,      swap_window_9},
  {MOD_NONE,  XK_h,      focus_window_left},
  {MOD_NONE,  XK_j,      focus_window_down},
  {MOD_NONE,  XK_k,      focus_window_up},
  {MOD_NONE,  XK_l,      focus_window_right},
  {MOD_SHIFT, XK_h,      swap_window_left},
  {MOD_SHIFT, XK_j,      swap_window_down},
  {MOD_SHIFT, XK_k,      swap_window_up},
  {MOD_SHIFT, XK_l,      swap_window_right},
  {MOD_NONE,  XK_q,      destroy_current_window},
  {MOD_NONE,  XK_Escape, insert_mode},
  {MOD_NONE,  XK_i,      insert_mode},
  {MOD_NONE,  XK_Return, terminal},
  {MOD_NONE,  XK_Left,   shrink_width},
  {MOD_NONE,  XK_Up,     shrink_height},
  {MOD_NONE,  XK_Right,  enlarge_width},
  {MOD_NONE,  XK_Down,   enlarge_height},
  {MOD_NONE,  XK_f,      librewolf}               //testing
};

size_t spawn_order[] = {
  0, 4, 6, 1, 7, 3, 5, 2
};

uint32_t gaps = 15;

int bar_height = 20;
char bar_color[6] = "111111";
char bar_font[] = "CodeNewRoman Nerd Font 12";
