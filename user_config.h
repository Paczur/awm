xcb_keysym_t normal_shortcut = XK_Super_L;

shortcut_t shortcuts[] = {
  {XK_2,      focus_window_1},
  {XK_1,      focus_window_0},
  {XK_3,      focus_window_2},
  {XK_4,      focus_window_3},
  {XK_5,      focus_window_4},
  {XK_6,      focus_window_5},
  {XK_7,      focus_window_6},
  {XK_8,      focus_window_7},
  {XK_9,      focus_window_8},
  {XK_0,      focus_window_9},
  {XK_q,      destroy_current_window},
  {XK_j,      focus_window_down},
  {XK_h,      focus_window_left},
  {XK_k,      focus_window_up},
  {XK_l,      focus_window_right},
  {XK_Escape, insert_mode},
  {XK_i,      insert_mode},
  {XK_Return, terminal},
  {XK_Left,   shrink_width},
  {XK_Up,     shrink_height},
  {XK_Right,  enlarge_width},
  {XK_Down,   enlarge_height},
  {XK_f,      librewolf}               //testing
};

size_t spawn_order[] = {
  0, 4, 6, 1, 7, 3, 5, 2
};

uint32_t gaps = 15;

int bar_height = 20;
char bar_color[6] = "111111";
char bar_font[] = "CodeNewRoman Nerd Font 12";
