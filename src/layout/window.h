#ifndef H_LAYOUT_WINDOW
#define H_LAYOUT_WINDOW

#include <stdbool.h>
#include <xcb/xcb.h>

#include "layout_types.h"

typedef struct window_list_t window_list_t;
typedef struct window_t window_t;

extern window_list_t *windows_minimized;
extern window_t *windows;
extern pthread_rwlock_t window_lock;

window_t *window_find(xcb_window_t);
window_t *window_minimized_nth(size_t n);
bool window_set_urgency(window_t *, bool);
bool window_set_input(window_t *, bool);
void window_show(const window_t *window);
void window_minimize_request(window_t *);
bool window_minimize_requested(const window_t *);
void window_minimize(window_t *);
void window_init(xcb_connection_t *, const char *const (*)[2], size_t,
                 xcb_get_property_reply_t *(*)(xcb_window_t, size_t));
void window_deinit(void);
void window_event_destroy(window_t *);
window_t *window_event_create(xcb_window_t);

#endif
