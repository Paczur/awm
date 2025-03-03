#ifndef H_AWM_BAR
#define H_AWM_BAR

#include "../types.h"

#define BAR_INACTIVE_BACKGROUND 0xff111111
#define BAR_INACTIVE_FOREGROUND 0xff333333
#define BAR_ACTIVE_BACKGROUND 0xff111111
#define BAR_ACTIVE_FOREGROUND 0xfff3f3f3
#define BAR_URGENT_BACKGROUND 0xff111111
#define BAR_URGENT_FOREGROUND 0xfff3f36e

#define BAR_INACTIVE 0
#define BAR_ACTIVE 1
#define BAR_URGENT 2

#define BAR_FLAGS_NONE 0
#define BAR_FLAGS_ALWAYS_ACTIVE (1 << 0)

#define BAR_WINDOW_NAME_LENGTH 5
#define BAR_FONT "-misc-custom-medium-r-normal--14-200-72-72-c-100-iso10646-1"
#define BAR_PADDING 5
#define BAR_OUTER_MARGIN 5
#define BAR_INNER_MARGIN 3
#define BAR_CLOCKED_BLOCKS                            \
  {                                                   \
    {"date +%H:%M", 60, BAR_FLAGS_ALWAYS_ACTIVE},     \
    {"date \"+%a %d\"", 60, BAR_FLAGS_ALWAYS_ACTIVE}, \
    {"/etc/awm/scripts/status", 5, BAR_FLAGS_NONE},   \
  }

void update_workspace(u32 *windows, u32 workspace);
void update_visible_workspaces(u32 *workspaces, u32 count);
void update_focused_monitor(u32 m);
void update_mode(u32 mode);
void update_minimized_windows(u32 *windows, u32 count);
void redraw_bar(void);

u32 get_bar_height(void);
void init_bar(const struct geometry *geoms, u32 monitor_count);
void deinit_bar(void);

#endif
