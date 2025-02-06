#ifndef H_AWM_LAYOUT
#define H_AWM_LAYOUT

#include "../types.h"

#define WINDOWS_PER_WORKSPACE 4
#define WORKSPACE_COUNT 10
#define MAX_MONITOR_COUNT 4

struct geometry {
  u32 x;
  u32 y;
  u32 width;
  u32 height;
};

void init_layout(const struct geometry *geoms, u32 monitor_count);

void map_request(u32 window);
void map_notify(u32 window);
void unmap_notify(u32 window);
void configure_notify(u32 window, struct geometry);
void destroy_notify(u32 window);
void focus_in_notify(u32 window);
void focus_out_notify(u32 window);

#endif

