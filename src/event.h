#ifndef H_EVENT
#define H_EVENT

#include <xcb/xcb.h>

void event_listener_add(uint8_t, void (*)(const xcb_generic_event_t*));
void event_next(xcb_connection_t*);

#endif
