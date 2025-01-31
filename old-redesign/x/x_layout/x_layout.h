#ifndef H_AWM_X_LAYOUT
#define H_AWM_X_LAYOUT

#include "../x_public.h"

x_size x_screen_size(void);

void x_map_window(x_window *);
void x_unmap_window(x_window *);
void x_focus_window(x_window *);
void x_destroy_window(x_window *, bool force);
void x_resize_window(x_window *, x_size);
void x_move_window_to_position(x_window *, x_position);

void x_set_window_minimized(x_window *, bool);
void x_set_window_urgent(x_window *, bool);
void x_set_window_fullscreen(x_window *, bool);
void x_set_workspace_empty(x_workspace *, bool);
void x_set_workspace_urgent(x_workspace *, bool);
void x_set_workspace_mode(x_workspace *, x_workspace_mode);
void x_set_window_geometry(x_window *, x_geometry);

x_workspace_mode x_get_workspace_mode(x_workspace *);

bool x_is_window_minimized(x_window *);
bool x_is_window_urgent(x_window *);
bool x_is_window_fullscreen(x_window *);
bool x_is_workspace_empty(x_workspace *);
bool x_is_workspace_urgent(x_workspace *);

const char *x_window_name(x_window *);
const char *x_workspace_name(x_workspace *);
x_window *x_focused_window(void);

#endif
