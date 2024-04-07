#ifndef H_LAYOUT_UNMANAGED
#define H_LAYOUT_UNMANAGED

#include <xcb/xcb.h>
#include "layout_types.h"

void unmanaged_init(xcb_connection_t*);
void unmanaged_deinit(void);

void unmanaged_event_map(xcb_window_t, const rect_t*);

#endif
