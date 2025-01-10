#include "x_private.h"

uint8_t xkb_event = -1;
uint8_t randr_event = -1;

xcb_visualtype_t *visual_type;
xcb_connection_t *conn;
const xcb_setup_t *setup;
xcb_screen_t *screen;
