#ifndef H_CONTROLLER
#define H_CONTROLLER

#include "types.h"
#include <stddef.h>
#include <xcb/xcb.h>

void c_shutdown(void);
void c_workspace_switch(size_t);
void c_window_show(size_t);
void c_window_focus(size_t);
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
void c_window_focused_destroy(void);
void c_window_focused_minimize(void);
void c_launcher_show(void);
void c_launcher_cancel(void);
void c_launcher_run(void);
void c_launcher_select_left(void);
void c_launcher_select_right(void);
void c_launcher_erase(void);
void c_mode_set(MODE);
void c_bar_block_update(size_t);
void c_bar_block_update_highlight(size_t, int);
void c_event_map(const xcb_generic_event_t*);
void c_event_key_press(const xcb_generic_event_t*);
void c_event_create(const xcb_generic_event_t*);
void c_event_destroy(const xcb_generic_event_t*);
void c_event_unmap(const xcb_generic_event_t*);
void c_event_focus(const xcb_generic_event_t*);
void c_event_expose(const xcb_generic_event_t*);
void c_loop(void);
void c_init(void);
void c_deinit(void);

#endif
