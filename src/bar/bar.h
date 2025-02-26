#ifndef H_AWM_BAR
#define H_AWM_BAR

#include "../types.h"

#define BAR_INACTIVE_BACKGROUND 0x111111
#define BAR_INACTIVE_FOREGROUND 0x333333
#define BAR_ACTIVE_BACKGROUND 0x111111
#define BAR_ACTIVE_FOREGROUND 0xffffff
#define BAR_URGENT_BACKGROUND 0x111111
#define BAR_URGENT_FOREGROUND 0xf3f36e

#define BAR_INACTIVE 0
#define BAR_ACTIVE 1
#define BAR_URGENT 2

#define BAR_FONT "-misc-custom-medium-r-normal--14-200-72-72-c-100-iso10646-1"
#define BAR_PADDING 5

void update_workspace(u32 *windows, u32 workspace);
void update_visible_workspaces(u32 *workspaces, u32 count);
void init_bar(const struct geometry *geoms, u32 monitor_count);

#endif
