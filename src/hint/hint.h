#ifndef H_HINT
#define H_HINT

#include "hint_types.h"
#include <xcb/xcb.h>
#include <stdbool.h>

bool hint_window_delete(xcb_window_t);
xcb_get_property_reply_t *hint_window_class(xcb_window_t, size_t);
void hint_window_update_state(xcb_window_t, WINDOW_STATE, WINDOW_STATE);
void hint_window_hints_set(xcb_window_t);
void hint_workspace_focused_set(size_t workspace);
void hint_window_focused_set(xcb_window_t);
void hint_frame_extents_set(xcb_window_t);
bool hint_initial_state_normal(xcb_window_t);
bool hint_state_iconic(uint32_t);
bool hint_atom_wm_change_state(xcb_atom_t);
bool hint_atom_close_window(xcb_atom_t);
bool hint_atom_frame_extents(xcb_atom_t);
bool hint_atom_urgent(xcb_atom_t);
bool hint_atom_input(xcb_atom_t);
size_t hint_atom_window_type(xcb_atom_t**, xcb_window_t);
bool hint_window_input(xcb_window_t);
bool hint_window_urgent(xcb_window_t, xcb_atom_t);
bool hint_atom_window_type_splash(xcb_atom_t);
size_t hint_saved_windows(xcb_window_t**);
size_t hint_saved_window_workspace(xcb_window_t);
size_t hint_saved_workspace_focused(void);
xcb_window_t hint_saved_window_focused(void);
void hint_init(const hint_init_t*);
void hint_deinit(void);

#endif
