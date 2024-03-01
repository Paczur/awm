#ifndef H_HINT
#define H_HINT

#include "hint_types.h"
#include <xcb/xcb.h>
#include <stdbool.h>

bool hint_delete_window(xcb_window_t);
xcb_get_property_reply_t *hint_window_class(xcb_window_t, size_t);
void hint_update_state(xcb_window_t, WINDOW_STATE);
void hint_set_window_hints(xcb_window_t);
bool hint_is_wm_change_state(xcb_atom_t);
bool hint_is_iconic_state(uint32_t);

void hint_init(xcb_connection_t*, const xcb_screen_t*);
void hint_deinit(void);

#endif
