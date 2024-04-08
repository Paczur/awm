#ifndef H_CONTROLLER
#define H_CONTROLLER

#include "types.h"
#include <stddef.h>
#include <xcb/xcb.h>
#include <stdbool.h>

void c_wm_shutdown(void);
void c_workspace_switch(size_t);
void c_workspace_fullscreen(size_t, size_t);
void c_workspace_focused_fullscreen(void);
void c_window_destroy(xcb_window_t, bool);
void c_window_show(size_t);
void c_window_focus(size_t);
bool c_window_minimize(xcb_window_t);
xcb_window_t c_window_focused(void);
void c_window_focused_swap(size_t);
void c_window_focus_down(void);
void c_window_focus_up(void);
void c_window_focus_left(void);
void c_window_focus_right(void);
void c_window_focused_swap_down(void);
void c_window_focused_swap_up(void);
void c_window_focused_swap_left(void);
void c_window_focused_swap_right(void);
void c_window_focused_resize_w(int);
void c_window_focused_resize_h(int);
void c_window_focused_reset_size(void);
void c_run(const char*);
void c_window_focused_destroy(bool);
void c_window_focused_minimize(void);
void c_launcher_show(void);
void c_launcher_cancel(void);
void c_launcher_run(void);
void c_launcher_select_left(void);
void c_launcher_select_right(void);
void c_launcher_erase(void);
void c_mode_set(MODE);
void c_mode_delay(MODE);
void c_mode_force(void);
void c_bar_block_update(size_t);
void c_bar_block_update_highlight(size_t, int);
void c_loop(void);
void c_init(void);
void c_deinit(void);

#endif
