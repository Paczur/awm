#ifndef H_AWM_BAR
#define H_AWM_BAR

#include "../const.h"
#include "../types.h"

struct launcher_control {
  u32 keysym;
  void (*f)(void);
};

void update_urgent_workspaces(
  u32 windows[WORKSPACE_COUNT][WINDOWS_PER_WORKSPACE]);
void update_workspaces(u32 windows[WORKSPACE_COUNT][WINDOWS_PER_WORKSPACE]);
void update_urgent_minimized(u32 windows[MINIMIZE_QUEUE_SIZE]);
void update_visible_workspaces(u32 *workspaces, u32 count);
void update_focused_monitor(u32 m);
void update_mode(u32 mode);
void update_minimized_window_name(u32 window);
void update_minimized_windows(u32 *windows, u32 count);
void update_clocked_block(u32 id);
void redraw_bar(void);

void launcher_handle_key(u8 flags, u8 keycode);
void show_launcher(void);
void hide_launcher(void);
u32 launcher_showing(void);
void launcher_run(void);
void launcher_hint_left(void);
void launcher_hint_right(void);
void launcher_erase(void);
u32 get_bar_height(void);
void init_bar(const struct geometry *geoms, u32 m_count);
void deinit_bar(void);
void bar_visibility(u32 val);
u32 bar_block_press(u32 window, u32 button);
void update_bar_colorscheme(void);

#endif
