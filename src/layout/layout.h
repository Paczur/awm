#ifndef H_AWM_LAYOUT
#define H_AWM_LAYOUT

#include "../types.h"
#define BORDER_SIZE 3
#define GAP_SIZE 5
#define BORDER_FOCUSED 0xfff3f3f3
#define BORDER_UNFOCUSED 0xff111111

void init_layout(const struct geometry *normal,
                 const struct geometry *fullscreen, u32 monitor_count);

void map_request(u32 window);
void unmap_notify(u32 window);
void destroy_notify(u32 window);
void focus_in_notify(u32 window);
void focus_out_notify(u32 window);
void destroy_notify(u32 window);

void focus_window(u32 window);
void focus_window_to_left(void);
void focus_window_to_right(void);
void focus_window_above(void);
void focus_window_below(void);

void swap_windows_by_index(u32 n);

void swap_focused_window_with_right(void);
void swap_focused_window_with_left(void);
void swap_focused_window_with_above(void);
void swap_focused_window_with_below(void);

void reset_layout_state(void);

void delete_focused_window(void);

void change_workspace(u32 w);
void minimize_focused_window(void);
void unminimize_window(u32 index);
void clean_layout_state(void);
u32 is_workspace_empty(void);
void restore_focus(void);

void change_size_offset(i32 x, i32 y);
void reset_size_offset(void);

void toggle_fullscreen_window(void);

#endif

