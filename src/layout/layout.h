#ifndef H_AWM_LAYOUT
#define H_AWM_LAYOUT

#include "../x/x.h"

typedef enum layout_mode {
  LAYOUT_MODE_TILING,
  LAYOUT_MODE_FLOATING,
} layout_mode;

typedef struct layout_window layout_window;
typedef struct layout_workspace layout_workspace;

/* Global */
uint32_t layout_screen_width(void);
uint32_t layout_screen_height(void);

/* Windows */
layout_window *layout_window_by_order(uint32_t);
layout_window *layout_top_left_window(const layout_workspace *);
layout_window *layout_top_right_window(const layout_workspace *);
layout_window *layout_bottom_left_window(const layout_workspace *);
layout_window *layout_bottom_right_window(const layout_workspace *);
layout_window *layout_window_to_left_of(const layout_window *, bool wrap);
layout_window *layout_window_to_right_of(const layout_window *, bool wrap);
layout_window *layout_window_above(const layout_window *, bool wrap);
layout_window *layout_window_bellow(const layout_window *, bool wrap);
layout_window *layout_focused_window(void);

layout_window *layout_window_from_x(x_window);
x_window layout_window_to_x(const layout_window *);

uint32_t layout_count_all_windows(void);
bool layout_is_window_minimized(const layout_window *);
bool layout_is_window_fullscreen(const layout_window *);
bool layout_is_window_urgent(const layout_window *);
layout_workspace *layout_window_location(const layout_window *);
const char *layout_window_name(const layout_window *);
uint32_t layout_window_width(const layout_window *);
uint32_t layout_window_heigth(const layout_window *);
uint32_t layout_window_x(const layout_window *);
uint32_t layout_window_y(const layout_window *);

awm_status layout_focus_window(layout_window *);
awm_status layout_resize_window(layout_window *, int32_t x, int32_t y);
awm_status layout_reset_window_size(layout_window *);
awm_status layout_destroy_window(layout_window *, bool force);
awm_status layout_swap_windows(layout_window *, layout_window *);
awm_status layout_window_move(layout_window *, uint32_t x, uint32_t y);
awm_status layout_render_window_at(layout_window *, uint32_t x, uint32_t y);
awm_status layout_move_window_to_workspace(layout_window *, layout_workspace *);
awm_status layout_window_urgent_set(layout_window *, bool);
awm_status layout_window_toggle_fullscreen(layout_window *);
awm_status layout_window_fullscreen_set(layout_window *, bool);
awm_status layout_window_minimize_set(layout_window *, bool);

awm_status layout_create_window(x_window);

/* Workspaces */
layout_workspace *layout_workspace_by_order(uint32_t);
layout_workspace *layout_workspace_next(const layout_workspace *);
layout_workspace *layout_workspace_prev(const layout_workspace *);
layout_workspace *layout_workspace_focused(void);

const char *layout_workspace_name(const layout_workspace *);
bool layout_workspace_isempty(const layout_workspace *);
bool layout_workspace_isurgent(const layout_workspace *);
uint32_t layout_workspace_window_count(const layout_workspace *);
uint32_t layout_workspace_count(void);
layout_mode layout_workspace_mode(const layout_workspace *);

awm_status layout_workspace_mode_set(layout_workspace *, layout_mode);
awm_status layout_workspace_mode_toggle(layout_workspace *);

#endif
