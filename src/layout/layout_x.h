#ifndef H_AWM_LAYOUT_X
#define H_AWM_LAYOUT_X

#include "../types.h"
#include "../x/x_p.h"

void send_current_workspace(u32 workspace);
u32 query_current_workspace(void);
void query_workspaces(u32 *workspaces);
void send_workspace(u32 *windows, u32 w);

void map_window(u32 window);
void unmap_window(u32 window);
void configure_window(u32 window, u32 x, u32 y, u32 width, u32 heigth);

#endif
