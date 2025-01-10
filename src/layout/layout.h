#ifndef H_AWM_LAYOUT
#define H_AWM_LAYOUT

#include "../x/x.h"

typedef uint32_t layout_order;
typedef struct layout_window layout_window;
typedef struct layout_workspace layout_workspace;

layout_window *layout_window_by_x_window(x_window);
layout_window *layout_window_by_order(layout_order);
layout_window *layout_top_left_window(const layout_workspace *);
layout_window *layout_top_right_window(const layout_workspace *);
layout_window *layout_bottom_left_window(const layout_workspace *);
layout_window *layout_bottom_right_window(const layout_workspace *);
layout_window *layout_window_to_left_of(const layout_window *, bool wrap);
layout_window *layout_window_to_right_of(const layout_window *, bool wrap);
layout_window *layout_window_above(const layout_window *, bool wrap);
layout_window *layout_window_bellow(const layout_window *, bool wrap);

layout_workspace *layout_workspace_by_order(layout_order);
layout_workspace *layout_next_workspace(const layout_workspace *);
layout_workspace *layout_prev_workspace(const layout_workspace *);

x_window layout_window_to_x_window(const layout_window *);

layout_workspace *layout_window_location(const layout_window *);

awm_status layout_focus_window(layout_window *);
awm_status layout_resize_window(layout_window *, x_size);
awm_status layout_reset_window_size(layout_window *);
awm_status layout_destroy_window(layout_window *, bool force);
awm_status layout_swap_window_positions(layout_window *, layout_window *);
awm_status layout_move_window_to_position(layout_window *, x_position);
awm_status layout_move_window_to_workspace(layout_window *, layout_workspace *);
awm_status layout_preview_window_at(layout_window *, x_position);
awm_status layout_stop_preview(layout_window *);
awm_status layout_minimize_window(layout_window *);

awm_status layout_workspace_set_mode(layout_workspace *, x_workspace_mode);
awm_status layout_window_set_urgent(layout_window *);
awm_status layout_window_set_fullscreen(layout_window *, bool);
awm_status layout_window_toggle_fullscreen(layout_window *);

#endif
