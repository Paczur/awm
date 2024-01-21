xcb_keysym_t normal_shortcut = XK_Super_L;
shortcut_t shortcuts[] = {
  {XK_Escape, insert_mode},
  {XK_i, insert_mode},
  {XK_Return, terminal}
};

size_t spawn_order[] = {
  0, 4, 7, 1, 6, 2, 5, 3
};
