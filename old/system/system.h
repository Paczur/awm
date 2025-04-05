#ifndef H_SYSTEM
#define H_SYSTEM

#include <stddef.h>
#include <xcb/xcb.h>

#include "system_types.h"

// XCB
extern xcb_visualtype_t *visual_type;
extern xcb_connection_t *conn;
extern const xcb_setup_t *setup;
extern xcb_screen_t *screen;

int system_sh_out(const char *, char *, size_t);
void system_sh(const char *);
void system_monitors(rect_t **monitors, size_t *monitor_count);
uint8_t system_xkb(void);
uint8_t system_randr(void);
void system_init(void (*term_action)(int));
void system_deinit(void);

#endif
