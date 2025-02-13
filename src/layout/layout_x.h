#ifndef H_AWM_LAYOUT_X
#define H_AWM_LAYOUT_X

#include "../types.h"
#include "../x/x_p.h"

void send_visible_workspaces(u32 *workspaces, u32 count);
void query_visible_workspaces(u32 *workspaces, u32 count);
void query_workspaces(u32 *workspaces);
void send_workspace(u32 *windows, u32 w);

void map_window(u32 window);
void unmap_window(u32 window);
void configure_window(u32 window, u32 x, u32 y, u32 width, u32 heigth,
                      u32 border);
void listen_to_events(u32 window);
void change_window_border_color(u32 window, u32 color);
void send_focused_window(u32 window);
u32 query_focused_window(void);
void focus_window(u32 window);
i32 is_root(u32 window);
void delete_window(u32 window);

#endif
