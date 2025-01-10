#ifndef H_AWM_X
#define H_AWM_X

#include "x_layout/x_layout.h"
#include "x_public.h"
#include "x_shortcut/x_shortcut.h"

void x_init(void);
void x_deinit(void);

/* EVENTS */
x_event *x_event_next(x_event *prev);
uint32_t x_event_type(const x_event *);
x_window *x_event_window(const x_event *);
uint8_t x_event_error_code(const x_event *);

void x_free(void *);

#endif
