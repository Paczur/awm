#define CONFIG_NUMBER_PATTERN(mod, f) \
{mod, XK_1, f ## _0}, \
{mod, XK_2, f ## _1}, \
{mod, XK_3, f ## _2}, \
{mod, XK_4, f ## _3}, \
{mod, XK_5, f ## _4}, \
{mod, XK_6, f ## _5}, \
{mod, XK_7, f ## _6}, \
{mod, XK_8, f ## _7}, \
{mod, XK_9, f ## _8}, \
{mod, XK_0, f ## _9}
#define CONFIG_DIRECTION_PATTERN(mod, f) \
{mod, XK_h, f ## _left}, \
{mod, XK_j, f ## _down}, \
{mod, XK_k, f ## _up},   \
{mod, XK_l, f ## _right}

#define CONFIG_SHORTCUTS_NORMAL_MODE { \
  CONFIG_NUMBER_PATTERN(MOD_NONE,     grid_focus), \
  CONFIG_NUMBER_PATTERN(MOD_SHIFT,    grid_swap),  \
  CONFIG_NUMBER_PATTERN(MOD_ALT,      workspace),    \
  CONFIG_NUMBER_PATTERN(MOD_CTRL,     show),         \
  CONFIG_DIRECTION_PATTERN(MOD_NONE,  focus_window), \
  CONFIG_DIRECTION_PATTERN(MOD_SHIFT, swap_window),  \
  CONFIG_DIRECTION_PATTERN(MOD_ALT,   enlarge),      \
  {MOD_NONE,  XF86XK_MonBrightnessDown, brightness_down}, \
  {MOD_NONE,  XF86XK_MonBrightnessUp,   brightness_up},   \
  {MOD_NONE,  XK_F1,                    brightness_down}, \
  {MOD_NONE,  XK_F2,                    brightness_up},   \
  {MOD_NONE,  XF86XK_AudioMute,         volume_mute},     \
  {MOD_NONE,  XF86XK_AudioLowerVolume,  volume_down},     \
  {MOD_NONE,  XF86XK_AudioRaiseVolume,  volume_up},       \
  {MOD_SHIFT, XK_s,                     system_suspend},  \
  {MOD_ALT,   XK_s,                     system_shutdown}, \
  {MOD_NONE,  XK_F4,                    volume_mute},     \
  {MOD_NONE,  XK_F5,                    volume_down},     \
  {MOD_NONE,  XK_F6,                    volume_up},       \
  {MOD_NONE,  XK_q,                     destroy},         \
  {MOD_SHIFT, XK_q,                     shutdown},        \
  {MOD_NONE,  XK_m,                     minimize},        \
  {MOD_NONE,  XK_Escape,                insert_mode},     \
  {MOD_NONE,  XK_i,                     insert_mode},     \
  {MOD_NONE,  XK_equal,                 equal_sizes},     \
  {MOD_NONE,  XK_Return,                terminal},        \
  {MOD_NONE,  XK_f,                     librewolf},       \
  {MOD_NONE,  XK_r,                     launch}           \
}
#define CONFIG_SHORTCUTS_INSERT_MODE { \
  {MOD_NONE, XK_Super_L, normal_mode}, \
}

#define CONFIG_SHORTCUTS_LAUNCHER { \
  {MOD_NONE, XK_Super_L, launcher_abort}, \
  {MOD_NONE, XK_Escape, launcher_abort}, \
  {MOD_NONE, XK_Return, launcher_confirm}, \
  {MOD_NONE, XK_Left, launcher_hint_left}, \
  {MOD_NONE, XK_Right, launcher_hint_right}, \
  {MOD_NONE, XK_BackSpace, launcher_erase}, \
}

#define CONFIG_TERMINAL_CMD "mlterm"
#define CONFIG_RESIZE_STEP 10

#define CONFIG_SPAWN_ORDER { 0, 4, 6, 1, 7, 3, 5, 2 }
#define CONFIG_GAPS 20

#define CONFIG_BAR_HEIGHT 32
#define CONFIG_BAR_FONT "CodeNewRoman Nerd Font Propo 14"
#define CONFIG_BAR_COMPONENT_PADDING 8
#define CONFIG_BAR_COMPONENT_SEPARATOR 2
#define CONFIG_BAR_MODE_MIN_WIDTH 32
#define CONFIG_BAR_WORKSPACE_MIN_WIDTH 32

#define CONFIG_BAR_BACKGROUND "111111"
#define CONFIG_BAR_MODE_INSERT_BACKGROUND "111111"
#define CONFIG_BAR_MODE_INSERT_FOREGROUND "333333"
#define CONFIG_BAR_MODE_NORMAL_BACKGROUND "111111"
#define CONFIG_BAR_MODE_NORMAL_FOREGROUND "FFFFFF"

#define CONFIG_BAR_WORKSPACE_FOCUSED_BACKGROUND "111111"
#define CONFIG_BAR_WORKSPACE_FOCUSED_FOREGROUND "FFFFFF"
#define CONFIG_BAR_WORKSPACE_UNFOCUSED_BACKGROUND "111111"
#define CONFIG_BAR_WORKSPACE_UNFOCUSED_FOREGROUND "333333"

#define CONFIG_BAR_MINIMIZED_ODD_BACKGROUND "111111"
#define CONFIG_BAR_MINIMIZED_ODD_FOREGROUND "FFFFFF"
#define CONFIG_BAR_MINIMIZED_EVEN_BACKGROUND "111111"
#define CONFIG_BAR_MINIMIZED_EVEN_FOREGROUND "333333"
#define CONFIG_BAR_MINIMIZED_MIN_WIDTH 32
#define CONFIG_BAR_MINIMIZED_NAME_MAX_LENGTH 40
#define CONFIG_BAR_MINIMIZED_NAME_REPLACEMENTS { \
  {"mlterm", "󰆍"}, \
  {"librewolf", ""}, \
  {"gimp", ""}, \
  {"telegram-desktop", ""}, \
  {0} \
}

#define CONFIG_BAR_INFO_MIN_WIDTH 0
#define CONFIG_BAR_INFO_BACKGROUND "111111"
#define CONFIG_BAR_INFO_FOREGROUND "333333"
#define CONFIG_BAR_INFO_HIGHLIGHTED_BACKGROUND "111111"
#define CONFIG_BAR_INFO_HIGHLIGHTED_FOREGROUND "FFFFFF"
#define CONFIG_BAR_INFO_BLOCKS { \
  {0, "/home/paczur/.config/i3blocks/net", 60}, \
  {1, "/home/paczur/.config/i3blocks/volume", -1}, \
  {2, "/home/paczur/.config/i3blocks/dnd", -1}, \
  {3, "echo  $(/home/paczur/.config/i3blocks/brightness)", -1}, \
  {4, "/home/paczur/.config/i3blocks/battery", 60}, \
  {5, "echo  $(/home/paczur/.config/i3blocks/cpu)", 5}, \
  {6, "date '+󰃶 %a %d' && exit 1", 3600}, \
  {7, "date '+ %H:%M' && exit 1", 30}, \
}

#define CONFIG_BAR_LAUNCHER_PROMPT_MIN_WIDTH 0
#define CONFIG_BAR_LAUNCHER_PROMPT_FOREGROUND "333333"
#define CONFIG_BAR_LAUNCHER_PROMPT_BACKGROUND "111111"

#define CONFIG_BAR_LAUNCHER_HINT_MIN_WIDTH 40
#define CONFIG_BAR_LAUNCHER_HINT_FOREGROUND "333333"
#define CONFIG_BAR_LAUNCHER_HINT_BACKGROUND "111111"

#define CONFIG_BAR_LAUNCHER_HINT_SELECTED_FOREGROUND "FFFFFF"
#define CONFIG_BAR_LAUNCHER_HINT_SELECTED_BACKGROUND "111111"