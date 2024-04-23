#include "event.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static void (**event_dispatch)(const xcb_generic_event_t*);
static size_t event_dispatch_length;

void event_listener_add(uint8_t type, void (*f)(const xcb_generic_event_t*)) {
  if(event_dispatch == NULL && XCB_GE_GENERIC < type) {
    event_dispatch =
    calloc(type + 1, sizeof(void (*)(const xcb_generic_event_t*)));
    event_dispatch_length = type + 1;
  } else if(event_dispatch == NULL) {
    event_dispatch =
    calloc(XCB_GE_GENERIC, sizeof(void (*)(const xcb_generic_event_t*)));
    event_dispatch_length = XCB_GE_GENERIC;
  } else if(type >= event_dispatch_length) {
    event_dispatch = realloc(
    event_dispatch, (type + 1) * sizeof(void (*)(const xcb_generic_event_t*)));
    event_dispatch_length = type + 1;
  }
  event_dispatch[type] = f;
}

void event_next(xcb_connection_t* conn) {
  xcb_generic_event_t* event;
  uint8_t type;
  event = xcb_wait_for_event(conn);
  if(event == NULL) return;
  type = event->response_type & 0x7F;  // Mask highbit - synthetic
  if(type < event_dispatch_length && event_dispatch[type] != NULL) {
    event_dispatch[type](event);
  }
  free(event);
}
