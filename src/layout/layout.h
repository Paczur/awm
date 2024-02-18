#ifndef H_LAYOUT
#define H_LAYOUT

#include <xcb/xcb.h>

void layout_focus_pick(void);

void layout_show(size_t);
void layout_minimize(void);
void layout_destroy(void);

void layout_init(void);
void layout_deinit(void);

void layout_event_map(xcb_window_t);
void layout_event_unmap(xcb_window_t);
void layout_event_create(xcb_window_t);
int layout_event_destroy(xcb_window_t);
void layout_event_focus(xcb_window_t);

#endif
