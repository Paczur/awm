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
layout_window *layout_window_top_left(void);
layout_window *layout_window_top_right(void);
layout_window *layout_window_bottom_left(void);
layout_window *layout_window_bottom_right(void);
layout_window *layout_window_to_left(const layout_window *, bool wrap);
layout_window *layout_window_to_right(const layout_window *, bool wrap);
layout_window *layout_window_above(const layout_window *, bool wrap);
layout_window *layout_window_bellow(const layout_window *, bool wrap);
layout_window *layout_window_focused(void);

layout_window *layout_window_from_awm(x_window);
x_window layout_window_to_awm(const layout_window *);

uint32_t layout_window_count(void);
bool layout_window_isminimized(const layout_window *);
bool layout_window_isfullscreen(const layout_window *);
bool layout_window_isurgent(const layout_window *);
const char *layout_window_name(const layout_window *);
layout_mode layout_window_mode(const layout_window *);
uint32_t layout_window_width(const layout_window *);
uint32_t layout_window_heigth(const layout_window *);
uint32_t layout_window_x(const layout_window *);
uint32_t layout_window_y(const layout_window *);

awm_status layout_window_focus(layout_window *);
awm_status layout_window_resize(layout_window *, int32_t x, int32_t y);
awm_status layout_window_size_reset(layout_window *);
awm_status layout_window_destroy(layout_window *, bool force);
awm_status layout_window_urgent_set(layout_window *, bool);
awm_status layout_window_fullscreen_toggle(layout_window *);
awm_status layout_window_fullscreen_set(layout_window *, bool);
awm_status layout_window_minimize_set(layout_window *, bool);
awm_status layout_window_swap(layout_window *, layout_window *);
awm_status layout_window_move(layout_window *, uint32_t x, uint32_t y);
awm_status layout_window_render_at(layout_window *, uint32_t x, uint32_t y);
awm_status layout_window_workspace_set(layout_window *, layout_workspace *);
awm_status layout_window_mode_set(layout_window *, layout_mode);
awm_status layout_window_mode_toggle(layout_window *);
awm_status layout_window_new(x_window);

/* Workspaces */
layout_workspace *layout_workspace_by_order(uint32_t);
layout_workspace *layout_workspace_next(const layout_workspace *);
layout_workspace *layout_workspace_prev(const layout_workspace *);
layout_workspace *layout_workspace_focused(void);

const char *layout_workspace_name(const layout_workspace *);
bool layout_workspace_isempty(const layout_workspace *);
bool layout_workspace_isurgent(const layout_workspace *);
layout_mode layout_workspace_mode(const layout_workspace *);
uint32_t layout_workspace_window_count(const layout_workspace *);
uint32_t layout_workspace_count(void);

awm_status layout_workspace_mode_set(layout_workspace *, layout_mode);
awm_status layout_workspace_mode_toggle(layout_workspace *);

#endif
