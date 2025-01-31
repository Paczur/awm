#ifndef H_AWM_X_LAYOUT_P
#define H_AWM_X_LAYOUT_P

#include "../x_private.h"
#include "x_layout.h"

void x_layout_init(void);
void x_layout_deinit(void);

x_window *x_window_from_xcb(xcb_window_t win);

#endif
