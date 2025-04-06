#ifndef H_AWM_LAYOUT_X
#define H_AWM_LAYOUT_X

#include "../types.h"
#include "../x/x_p.h"
#include "layout_config.h"

#define WINDOW_STATE_NONE 0
#define WINDOW_STATE_FULLSCREEN 1
#define WINDOW_STATE_URGENT 2

u32 requested_window_state(u32 window);
void map_window(u32 window);
void unmap_window(u32 window);
void configure_and_raise(u32 window, u32 x, u32 y, u32 width, u32 height,
                         u32 border);
void configure_window(u32 window, u32 x, u32 y, u32 width, u32 heigth,
                      u32 border);
void resize_window(u32 window, u32 x, u32 y, u32 width, u32 heigth);
void query_window_geometry(struct geometry *geom, u32 window);
void listen_to_events(u32 window);
void change_window_border_color(u32 window, u32 color);
void focus_window(u32 window);
void unfocus_window(void);
void delete_window(u32 window);
void delete_sent_layout_data(void);

void send_visible_workspaces(u32 *workspaces, u32 count);
void query_visible_workspaces(u32 *workspaces, u32 count);
void query_workspaces(u32 workspaces[WORKSPACE_COUNT][WINDOWS_PER_WORKSPACE]);
void send_workspaces(u32 windows[WORKSPACE_COUNT][WINDOWS_PER_WORKSPACE]);
void send_focused_windows(i32 windows[WORKSPACE_COUNT]);
void query_focused_windows(i32 windows[WORKSPACE_COUNT]);
void send_focused_monitor(u32 monitor);
u32 query_focused_monitor(void);

void send_minimized_windows(u32 *windows, u32 len);
u32 query_minimized_window_count(void);
void query_minimized_windows(u32 *windows, u32 len);

void send_workspace_count(u32 count);
void send_focused_workspace(u32 w);
void send_focused_window(u32 window);
void send_unfocused_window(u32 window);

void query_size_offsets(i32 *offsets);
void send_size_offsets(i32 *offsets);

void query_fullscreen_windows(u32 windows[WORKSPACE_COUNT]);
void send_fullscreen_windows(u32 windows[WORKSPACE_COUNT]);

void query_urgent_workspace_windows(
  u32 windows[WORKSPACE_COUNT][WINDOWS_PER_WORKSPACE]);
void send_urgent_workspace_windows(
  u32 windows[WORKSPACE_COUNT][WINDOWS_PER_WORKSPACE]);
void query_urgent_minimized_windows(u32 windows[MINIMIZE_QUEUE_SIZE]);
void send_urgent_minimized_windows(u32 windows[MINIMIZE_QUEUE_SIZE]);
void query_floating_workspaces(u32 workspaces[WORKSPACE_COUNT]);
void send_floating_workspaces(u32 workspaces[WORKSPACE_COUNT]);

u32 query_window_urgent(u32 window);

void set_window_fullscreen(u32 window);
void reset_window_fullscreen(u32 window);
void set_window_minimized(u32 window);
void reset_window_minimized(u32 window);

void bar_visibility(u32 val);

void setup_root(void);
void send_changes(void);

void raise_window(u32 window);

#endif
