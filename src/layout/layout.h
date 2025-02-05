#ifndef H_AWM_LAYOUT
#define H_AWM_LAYOUT

#include "../types.h"

void map_request(u32 window);
void map_notify(u32 window);
void unmap_notify(u32 window);
void configure_notify(u32 window, u16 x, u16 y, u16 width, u16 height);
void destroy_notify(u32 window);
void focus_in_notify(u32 window);
void focus_out_notify(u32 window);

#endif
