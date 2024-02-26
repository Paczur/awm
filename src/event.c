#include "event.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

static void(*event_dispatch[XCB_GE_GENERIC]) (const xcb_generic_event_t*);
static bool run;

void event_listener_add(uint8_t type, void(*f)(const xcb_generic_event_t*)) {
  event_dispatch[type] = f;
}

void event_run(xcb_connection_t *conn) {
  xcb_generic_event_t* event;
  run = true;
  while(run) {
    event = xcb_wait_for_event(conn);
    if(event->response_type < XCB_GE_GENERIC &&
       event_dispatch[event->response_type] != NULL) {
      event_dispatch[event->response_type](event);
      xcb_flush(conn);
    }
    free(event);
  }
}

void event_stop(void) { run = false; }
