#ifndef H_AWM_X_PRIVATE
#define H_AWM_X_PRIVATE

#include <xcb/randr.h>
#include <xcb/xcb.h>
#include <xcb/xkb.h>

#include "../log/log.h"
#include "../types.h"
#include "x_public.h"

extern uint8_t xkb_event;
extern uint8_t randr_event;

extern xcb_visualtype_t *visual_type;
extern xcb_connection_t *conn;
extern const xcb_setup_t *setup;
extern xcb_screen_t *screen;

#endif
