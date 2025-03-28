#ifndef H_AWM_LAYOUT
#define H_AWM_LAYOUT

#include "../types.h"

#define UP 0
#define ABOVE 0
#define LEFT 1
#define RIGHT 3
#define BELOW 4
#define DOWN 4

void init_layout(const struct geometry *normal,
                 const struct geometry *fullscreen, u32 monitor_count);

void map_request(u32 window);
void unmap_notify(u32 window);
void destroy_notify(u32 window);
void focus_in_notify(u32 window);
void focus_out_notify(u32 window);
void destroy_notify(u32 window);

void focus_window(u32 window);
void focus_window_direction(u32 direction);
void swap_windows_by_index(u32 n);
void swap_focused_window_with_direction(u32 direction);

void reset_layout_state(void);

void close_window(u32 window);
void close_focused_window(void);

void change_workspace(u32 w);
void set_minimized_window(u32 window, u32 state);
void minimize_focused_window(void);
void unminimize_window(u32 index);
void clean_layout_state(void);
u32 is_workspace_empty(void);
void restore_focus(void);

void change_size_offset(i32 x, i32 y);
void reset_size_offset(void);

void set_fullscreen_window(u32 window, u32 state);
void toggle_fullscreen_on_focused_window(void);

void update_layout_colorscheme(void);
void update_window_urgent(u32 window);

#endif

