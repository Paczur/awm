#ifndef H_AWM_BAR
#define H_AWM_BAR

#include "../types.h"

#define BAR_INACTIVE_BACKGROUND 0xff111111
#define BAR_INACTIVE_FOREGROUND 0xff333333
#define BAR_ACTIVE_BACKGROUND 0xff111111
#define BAR_ACTIVE_FOREGROUND 0xfff3f3f3
#define BAR_URGENT_BACKGROUND 0xff111111
#define BAR_URGENT_FOREGROUND 0xfff3f36e

#define BAR_LAUNCHER_HINT_BLOCKS 10
#define BAR_LAUNCHER_PROMPT_LENGTH 20
#define BAR_LAUNCHER_MAX_HINT_LENGTH 50

#define BAR_INACTIVE 0
#define BAR_ACTIVE 1
#define BAR_URGENT 2

#define BAR_FLAGS_NONE 0
#define BAR_FLAGS_ALWAYS_ACTIVE (1 << 0)

#define BAR_WINDOW_NAME_LENGTH 10
#define BAR_FONT "-misc-custom-medium-r-normal--14-200-72-72-c-100-iso10646-1"
#define BAR_PADDING 5
#define BAR_OUTER_MARGIN 5
#define BAR_INNER_MARGIN 3
#define BAR_INNER_INSIDE_MARGIN 1
#define BAR_CLOCKED_BLOCKS                                 \
  {                                                        \
    {"date +%H:%M", 60, BAR_FLAGS_ALWAYS_ACTIVE},          \
    {"date \"+%a %d\"", 60, BAR_FLAGS_ALWAYS_ACTIVE},      \
    {"/etc/awm/scripts/battery", 60, BAR_FLAGS_NONE},      \
    {"/etc/awm/scripts/volume", 3600, BAR_FLAGS_NONE},     \
    {"/etc/awm/scripts/brightness", 3600, BAR_FLAGS_NONE}, \
    {"/etc/awm/scripts/cpu", 30, BAR_FLAGS_ALWAYS_ACTIVE}, \
    {"/etc/awm/scripts/gpu", 30, BAR_FLAGS_ALWAYS_ACTIVE}, \
    {"/etc/awm/scripts/ram", 30, BAR_FLAGS_ALWAYS_ACTIVE}, \
  }

#define LAUNCHER_CONTROLS                                             \
  {                                                                   \
    {KEY_Escape, hide_launcher},      {KEY_Return, launcher_run},     \
    {KEY_BackSpace, launcher_erase},  {KEY_Left, launcher_hint_left}, \
    {KEY_Right, launcher_hint_right},                                 \
  }

struct launcher_control {
  u32 keysym;
  void (*f)(void);
};

void update_workspace(u32 *windows, u32 workspace);
void update_visible_workspaces(u32 *workspaces, u32 count);
void update_focused_monitor(u32 m);
void update_mode(u32 mode);
void update_minimized_windows(u32 *windows, u32 count);
void update_clocked_block(u32 id);
void redraw_bar(void);

void launcher_handle_key(u8 flags, u8 keycode);
void show_launcher(void);
void hide_launcher(void);
u32 launcher_showing(void);
void launcher_run(void);
void launcher_hint_left(void);
void launcher_hint_right(void);
void launcher_erase(void);
u32 get_bar_height(void);
void init_bar(const struct geometry *geoms, u32 m_count);
void deinit_bar(void);
void bar_visibility(u32 val);

#endif
