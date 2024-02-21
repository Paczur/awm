#ifndef H_LAYOUT_WINDOW
#define H_LAYOUT_WINDOW

#include <xcb/xcb.h>

#define WINDOW_NAME_MAX_LENGTH 40

typedef struct window_list_t window_list_t;
typedef struct window_t window_t;

extern window_list_t *windows_minimized;
extern window_t *windows;

window_t *window_find(xcb_window_t);
window_t *window_minimized_nth(size_t n);

void window_show(const window_t *window);
void window_minimize(window_t*);

void window_init(xcb_connection_t*, const char *const(*)[2]);
void window_deinit(void);

int window_event_destroy(xcb_window_t, window_t**);
void window_event_create(xcb_window_t);

#endif
