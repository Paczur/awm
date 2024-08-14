#define REPEATABLE(mod, key, f) {mod, key, f, true}
#define REPEATABLE_CHANGE(mod, key, f) \
  {mod, key, f, true}, { MOD_SUPER | mod, key, f##_change, false }
#define WITH_CHANGE(mod, key, f) \
  {mod, key, f, false}, { MOD_SUPER | mod, key, f##_change, false }
#define BARE(mod, key, f) {mod, key, f, false}
#define CONFIG_NUMBER_PATTERN(mod, f)                             \
  WITH_CHANGE(mod, XK_1, f##_0), WITH_CHANGE(mod, XK_2, f##_1),   \
    WITH_CHANGE(mod, XK_3, f##_2), WITH_CHANGE(mod, XK_4, f##_3), \
    WITH_CHANGE(mod, XK_5, f##_4), WITH_CHANGE(mod, XK_6, f##_5), \
    WITH_CHANGE(mod, XK_7, f##_6), WITH_CHANGE(mod, XK_8, f##_7), \
    WITH_CHANGE(mod, XK_9, f##_8), WITH_CHANGE(mod, XK_0, f##_9)
#define CONFIG_SHIFTED_NUMBER_PATTERN(mod, f)      \
  WITH_CHANGE(MOD_SHIFT, XK_exclam, f##_0),        \
    WITH_CHANGE(MOD_SHIFT, XK_at, f##_1),          \
    WITH_CHANGE(MOD_SHIFT, XK_numbersign, f##_2),  \
    WITH_CHANGE(MOD_SHIFT, XK_dollar, f##_3),      \
    WITH_CHANGE(MOD_SHIFT, XK_percent, f##_4),     \
    WITH_CHANGE(MOD_SHIFT, XK_asciicircum, f##_5), \
    WITH_CHANGE(MOD_SHIFT, XK_ampersand, f##_6),   \
    WITH_CHANGE(MOD_SHIFT, XK_asterisk, f##_7),    \
    WITH_CHANGE(MOD_SHIFT, XK_parenleft, f##_8),   \
    WITH_CHANGE(MOD_SHIFT, XK_parenright, f##_9)
#define CONFIG_DIRECTION_PATTERN(mod, f)                              \
  WITH_CHANGE(mod, XK_h, f##_left), WITH_CHANGE(mod, XK_j, f##_down), \
    WITH_CHANGE(mod, XK_k, f##_up), WITH_CHANGE(mod, XK_l, f##_right)
#define REPEATABLE_DIRECTION_PATTERN(mod, f) \
  REPEATABLE_CHANGE(mod, XK_h, f##_left),    \
    REPEATABLE_CHANGE(mod, XK_j, f##_down),  \
    REPEATABLE_CHANGE(mod, XK_k, f##_up),    \
    REPEATABLE_CHANGE(mod, XK_l, f##_right)

// clang-format off
#define CONFIG_SHORTCUTS_NORMAL_MODE                                      \
  {                                                                       \
    CONFIG_NUMBER_PATTERN(MOD_NONE,         workspace),           \
    CONFIG_SHIFTED_NUMBER_PATTERN(MOD_NONE, layout_swap_focused), \
    CONFIG_NUMBER_PATTERN(MOD_CTRL,         workspace),           \
    CONFIG_NUMBER_PATTERN(MOD_ALT,          show),                \
    CONFIG_DIRECTION_PATTERN(MOD_NONE,      focus_window),        \
    CONFIG_DIRECTION_PATTERN(MOD_SHIFT,     swap_window),         \
    REPEATABLE_DIRECTION_PATTERN(MOD_ALT,   enlarge),             \
    REPEATABLE(MOD_NONE,   XF86XK_MonBrightnessDown, brightness_down), \
    REPEATABLE(MOD_NONE,   XF86XK_MonBrightnessUp,   brightness_up),   \
    REPEATABLE(MOD_NONE,   XK_F1,                    brightness_down), \
    REPEATABLE(MOD_NONE,   XK_F2,                    brightness_up),   \
    REPEATABLE(MOD_NONE,   XF86XK_AudioMute,         volume_mute),     \
    REPEATABLE(MOD_NONE,   XF86XK_AudioLowerVolume,  volume_down),     \
    REPEATABLE(MOD_NONE,   XF86XK_AudioRaiseVolume,  volume_up),       \
    REPEATABLE(MOD_NONE,   XK_F4,                    volume_mute),     \
    REPEATABLE(MOD_NONE,   XK_F5,                    volume_down),     \
    REPEATABLE(MOD_NONE,   XK_F6,                    volume_up),       \
    WITH_CHANGE(MOD_SHIFT, XK_s,                     system_suspend),  \
    WITH_CHANGE(MOD_ALT,   XK_s,                     system_shutdown), \
    WITH_CHANGE(MOD_NONE,  XK_q,                     destroy),         \
    WITH_CHANGE(MOD_SHIFT, XK_q,                     destroy_force),   \
    WITH_CHANGE(MOD_ALT,   XK_q,                     shutdown),        \
    WITH_CHANGE(MOD_NONE,  XK_m,                     minimize),        \
    WITH_CHANGE(MOD_NONE,  XK_Escape,                insert_mode),     \
    WITH_CHANGE(MOD_NONE,  XK_i,                     insert_mode),     \
    WITH_CHANGE(MOD_NONE,  XK_equal,                 equal_sizes),     \
    WITH_CHANGE(MOD_NONE,  XK_Return,                terminal),        \
    WITH_CHANGE(MOD_NONE,  XK_f,                     fullscreen),      \
    WITH_CHANGE(MOD_NONE,  XK_p,                     screenshot),      \
    WITH_CHANGE(MOD_NONE,  XK_b,                     librewolf),       \
    BARE(MOD_NONE,         XK_r,                     launch),          \
    BARE(MOD_NONE,         XK_c,                     color_toggle),    \
  }
#define CONFIG_SHORTCUTS_NORMAL_MODE_RELEASE \
  { BARE(MOD_SUPER, XK_Super_L, mode_force), }
#define CONFIG_SHORTCUTS_INSERT_MODE \
{ \
  BARE(MOD_NONE, XK_Super_L, normal_mode), \
}
#define CONFIG_SHORTCUTS_INSERT_MODE_RELEASE { }
#define CONFIG_SHORTCUTS_LAUNCHER_MODE               \
  {                                                  \
    BARE(MOD_NONE, XK_Super_L, launcher_abort),      \
      BARE(MOD_NONE, XK_Escape, launcher_abort),     \
      BARE(MOD_NONE, XK_Return, launcher_confirm),   \
      BARE(MOD_NONE, XK_Left, launcher_hint_left),   \
      BARE(MOD_NONE, XK_Right, launcher_hint_right), \
      BARE(MOD_NONE, XK_BackSpace, launcher_erase),  \
  }
#define CONFIG_SHORTCUTS_LAUNCHER_MODE_RELEASE { }
// clang-format on

#define CONFIG_TERMINAL_CMD "mlterm"
#define CONFIG_RESIZE_STEP 10

#define BLACK {"000000", "f3f3f3"}
#define DARK_GRAY {"111111", "eeeeee"}
#define GRAY {"333333", "aaaaaa"}
#define YELLOW {"f3f36e", "b32d2e"}
#define WHITE {"f3f3f3", "000000"}

#define CONFIG_SPAWN_ORDER {0, 4, 6, 1, 7, 3, 5, 2}
#define CONFIG_GAPS 15
#define CONFIG_BORDERS 5
#define CONFIG_BORDER_NORMAL GRAY
#define CONFIG_BORDER_URGENT WHITE
#define CONFIG_WORKSPACE_NUMBERS_ONLY false

#define CONFIG_BAR_BACKGROUND DARK_GRAY
#define CONFIG_BAR_HEIGHT 32
#define CONFIG_BAR_FONT "CodeNewRoman Nerd Font Propo 14"
#define CONFIG_BAR_COMPONENT_PADDING 8
#define CONFIG_BAR_COMPONENT_SEPARATOR 2

#define CONFIG_BAR_MODE_MIN_WIDTH 32
#define CONFIG_BAR_MODE_INSERT_BACKGROUND DARK_GRAY
#define CONFIG_BAR_MODE_INSERT_FOREGROUND GRAY
#define CONFIG_BAR_MODE_NORMAL_BACKGROUND DARK_GRAY
#define CONFIG_BAR_MODE_NORMAL_FOREGROUND WHITE

#define CONFIG_BAR_WORKSPACE_FOLD false
#define CONFIG_BAR_WORKSPACE_MIN_WIDTH 32
#define CONFIG_BAR_WORKSPACE_UNFOCUSED_BACKGROUND DARK_GRAY
#define CONFIG_BAR_WORKSPACE_UNFOCUSED_FOREGROUND GRAY
#define CONFIG_BAR_WORKSPACE_FOCUSED_BACKGROUND DARK_GRAY
#define CONFIG_BAR_WORKSPACE_FOCUSED_FOREGROUND WHITE
#define CONFIG_BAR_WORKSPACE_URGENT_BACKGROUND DARK_GRAY
#define CONFIG_BAR_WORKSPACE_URGENT_FOREGROUND YELLOW

#define CONFIG_BAR_MINIMIZED_MIN_WIDTH 32
#define CONFIG_BAR_MINIMIZED_NAME_MAX_LENGTH 40
#define CONFIG_BAR_MINIMIZED_EVEN_BACKGROUND DARK_GRAY
#define CONFIG_BAR_MINIMIZED_EVEN_FOREGROUND GRAY
#define CONFIG_BAR_MINIMIZED_ODD_BACKGROUND DARK_GRAY
#define CONFIG_BAR_MINIMIZED_ODD_FOREGROUND WHITE
#define CONFIG_BAR_MINIMIZED_URGENT_BACKGROUND DARK_GRAY
#define CONFIG_BAR_MINIMIZED_URGENT_FOREGROUND YELLOW
// clang-format off
#define CONFIG_BAR_MINIMIZED_NAME_REPLACEMENTS                 \
  {                                                            \
    {"mlterm",           "󰆍"}, \
    {"librewolf",        ""}, \
    {"LibreWolf",        ""}, \
    {"gimp",             ""}, \
    {"telegram-desktop", ""}, \
    {"mpv",              ""}, \
    {"feh",              ""}  \
  }
// clang-format on

#define CONFIG_BAR_INFO_MIN_WIDTH 0
#define CONFIG_BAR_INFO_NORMAL_BACKGROUND DARK_GRAY
#define CONFIG_BAR_INFO_NORMAL_FOREGROUND GRAY
#define CONFIG_BAR_INFO_HIGHLIGHTED_BACKGROUND DARK_GRAY
#define CONFIG_BAR_INFO_HIGHLIGHTED_FOREGROUND WHITE
#define CONFIG_BAR_INFO_URGENT_BACKGROUND DARK_GRAY
#define CONFIG_BAR_INFO_URGENT_FOREGROUND YELLOW
// clang-format off
#define CONFIG_BAR_INFO_BLOCKS                                                 \
{                                                                            \
  {0, "/etc/awm/scripts/net",        60}, \
  {1, "/etc/awm/scripts/volume",     -1}, \
  {2, "/etc/awm/scripts/dnd",        -1}, \
  {3, "/etc/awm/scripts/brightness", -1}, \
  {4, "/etc/awm/scripts/battery",    60}, \
  {8, "/etc/awm/scripts/gpu",        60}, \
  {5, "/etc/awm/scripts/cpu",        5},  \
  {6, "date '+󰃶  %a %d' && exit 1",  3600}, \
  {7, "date '+  %H:%M' && exit 1",  30}, \
}
// clang-format on

#define CONFIG_BAR_LAUNCHER_PROMPT_MIN_WIDTH 0
#define CONFIG_BAR_LAUNCHER_PROMPT_FOREGROUND GRAY
#define CONFIG_BAR_LAUNCHER_PROMPT_BACKGROUND DARK_GRAY

#define CONFIG_BAR_LAUNCHER_HINT_MIN_WIDTH 40
#define CONFIG_BAR_LAUNCHER_HINT_NORMAL_FOREGROUND GRAY
#define CONFIG_BAR_LAUNCHER_HINT_NORMAL_BACKGROUND DARK_GRAY

#define CONFIG_BAR_LAUNCHER_HINT_SELECTED_FOREGROUND WHITE
#define CONFIG_BAR_LAUNCHER_HINT_SELECTED_BACKGROUND DARK_GRAY
