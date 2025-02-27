#ifndef H_AWM_LAYOUT
#define H_AWM_LAYOUT

#include "../types.h"
#define BORDER_SIZE 10
#define GAP_SIZE 10
#define BORDER_FOCUSED 0xffffffff
#define BORDER_UNFOCUSED 0xff333333

void init_layout(const struct geometry *geoms, u32 monitor_count);

void map_request(u32 window);
void unmap_notify(u32 window);
void destroy_notify(u32 window);
void focus_in_notify(u32 window);
void focus_out_notify(u32 window);

void focus_window(u32 window);
void focus_window_to_left(void);
void focus_window_to_right(void);
void focus_window_above(void);
void focus_window_below(void);

void swap_focused_window_with_right(void);
void swap_focused_window_with_left(void);
void swap_focused_window_with_above(void);
void swap_focused_window_with_below(void);

void reset_layout_state(void);

void delete_focused_window(void);

void change_workspace(u32 w);

#endif

