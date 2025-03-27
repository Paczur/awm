#ifndef H_AWM_BAR_CONFIG
#define H_AWM_BAR_CONFIG

#include "../config.h"

#define BAR_INACTIVE_BACKGROUND DARK_GRAY
#define BAR_INACTIVE_FOREGROUND GRAY
#define BAR_ACTIVE_BACKGROUND DARK_GRAY
#define BAR_ACTIVE_FOREGROUND WHITE
#define BAR_URGENT_BACKGROUND DARK_GRAY
#define BAR_URGENT_FOREGROUND YELLOW

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

#endif

