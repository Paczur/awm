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

#define CONFIG_NORMAL_SHORTCUT XK_Super_L
#define CONFIG_SHORTCUTS { \
  CONFIG_NUMBER_PATTERN(MOD_NONE,     focus_window), \
  CONFIG_NUMBER_PATTERN(MOD_SHIFT,    swap_window),  \
  CONFIG_NUMBER_PATTERN(MOD_ALT,      workspace),    \
  CONFIG_NUMBER_PATTERN(MOD_CTRL,     show),         \
  CONFIG_DIRECTION_PATTERN(MOD_NONE,  focus_window), \
  CONFIG_DIRECTION_PATTERN(MOD_SHIFT, swap_window),  \
  CONFIG_DIRECTION_PATTERN(MOD_ALT,   enlarge),      \
  {MOD_NONE,  XK_q,      destroy},     \
  {MOD_SHIFT, XK_q,      shutdown},    \
  {MOD_NONE,  XK_m,      minimize},    \
  {MOD_NONE,  XK_Escape, insert_mode}, \
  {MOD_NONE,  XK_i,      insert_mode}, \
  {MOD_NONE,  XK_equal,  equal_sizes}, \
  {MOD_NONE,  XK_Return, terminal},    \
  {MOD_NONE,  XK_f,      librewolf}    \
}

#define CONFIG_TERMINAL_CMD "mlterm"
#define CONFIG_RESIZE_STEP 10

#define CONFIG_SPAWN_ORDER { 0, 4, 6, 1, 7, 3, 5, 2 }
#define CONFIG_GAPS 15

#define CONFIG_BAR_HEIGHT 32
#define CONFIG_BAR_FONT "CodeNewRoman Nerd Font 14"
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
  {0} \
}
