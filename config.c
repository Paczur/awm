#include "config.h"
#include "window.h"
#include "global.h"
#include "user_config.h"
#include "bar.h"
#include <stdlib.h> //calloc
#include <string.h> //memmove
#include <stdio.h> //printf

#define X(pos) ((pos)%2)
#define Y(pos) ((pos)%4/2)
#define COMB(x, y) ((x)+(y)*2)

#define window_n(n) \
  void focus_window_ ## n (void) { \
    focus_window_n(n); \
  } \
  void swap_window_ ## n (void) { \
    swap_windows(n, view.workspaces[view.focus].focus); \
  } \
  void workspace_ ## n (void) { \
    workspace_n(n); \
    redraw_workspaces(); \
  } \
  void show_ ## n (void) { \
    show_n(n); \
    redraw_minimized(); \
  }


#define TIMES10(x) x(0) x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9)
TIMES10(window_n)

  void shutdown(void) { restart = true; }
void focus_window_down(void) { focus_window_n(window_below()); }
void focus_window_up(void) { focus_window_n(window_above()); }
void focus_window_left(void) { focus_window_n(window_to_left()); }
void focus_window_right(void) { focus_window_n(window_to_right()); }
void swap_window_down(void) {
  swap_windows(view.workspaces[view.focus].focus, window_below());
}
void swap_window_up(void) {
  swap_windows(view.workspaces[view.focus].focus, window_above());
}
void swap_window_left(void) {
  swap_windows(view.workspaces[view.focus].focus, window_to_left());
}
void swap_window_right(void) {
  swap_windows(view.workspaces[view.focus].focus, window_to_right());
}
void enlarge_right(void) {
  resize_w(view.workspaces[view.focus].focus/4, CONFIG_RESIZE_STEP);
}
void enlarge_down(void) {
  resize_h(view.workspaces[view.focus].focus/4, CONFIG_RESIZE_STEP);
}
void enlarge_left(void) {
  resize_w(view.workspaces[view.focus].focus/4, -CONFIG_RESIZE_STEP);
}
void enlarge_up(void) {
  resize_h(view.workspaces[view.focus].focus/4, -CONFIG_RESIZE_STEP);
}
void equal_sizes(void) { reset_cross(view.workspaces[view.focus].focus/4); }
void terminal(void) { sh(CONFIG_TERMINAL_CMD); }
void destroy(void) { destroy_n(view.workspaces[view.focus].focus); }
void minimize(void) { minimize_n(view.workspaces[view.focus].focus); }
void librewolf(void) { sh("lb"); }
void launch(void) { show_launcher(); }
void volume_mute(void) {
  sh("volume m");
  update_info_n_highlight(1, 1);
}
void volume_up(void) {
  sh("volume +");
  update_info_n_highlight(1, 1);
}
void volume_down(void) {
  sh("volume -");
  update_info_n_highlight(1, 1);
}
void brightness_down(void) {
  sh("xbacklight -dec 2");
  update_info_n_highlight(3, 1);
}
void brightness_up(void) {
  sh("xbacklight -inc 2");
  update_info_n_highlight(3, 1);
}

typedef struct shortcut_t {
  MODIFIER modifier;
  xcb_keysym_t keysym;
  void (*function) (void);
} shortcut_t;

void normal_mode(void) {
  internal_shortcut_t *sh;
  mode = MODE_NORMAL;
  xcb_ungrab_key(conn, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
  for(size_t i=0; i<shortcut_lookup_l; i++) {
    sh = shortcut_lookup[i];
    while(sh != NULL) {
      xcb_grab_key(conn, 1, screen->root, sh->mod_mask, i+shortcut_lookup_offset,
                   XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
      sh = sh->next;
    }
  }
  redraw_mode();
}

void insert_mode(void) {
  mode = MODE_INSERT;
  xcb_ungrab_key(conn, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
  xcb_grab_key(conn, 1, screen->root, 0, normal_code,
               XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
  redraw_mode();
}

void convert_shortcuts(const xcb_get_keyboard_mapping_reply_t *kmapping) {
  int start = 0;
  xcb_keycode_t index;
  internal_shortcut_t *sh;
  shortcut_t shortcuts[] = CONFIG_SHORTCUTS;

  normal_code = keysym_to_keycode(CONFIG_NORMAL_SHORTCUT, kmapping);

  shortcut_lookup_l = setup->max_keycode-setup->min_keycode;
  shortcut_lookup_offset = setup->min_keycode;
  shortcut_lookup = calloc(shortcut_lookup_l, sizeof(internal_shortcut_t*));

  // get keycodes from keysyms
  for(size_t i=0; i<LENGTH(shortcuts); i++) {
    index = keysym_to_keycode(shortcuts[i].keysym, kmapping) -
      shortcut_lookup_offset;
    if(shortcut_lookup[index] == NULL) {
      shortcut_lookup[index] = malloc(sizeof(internal_shortcut_t));
      sh = shortcut_lookup[index];
    } else {
      sh = shortcut_lookup[index];
      while(sh->next != NULL) sh = sh->next;
      sh->next = malloc(sizeof(internal_shortcut_t));
      sh = sh->next;
    }
    sh->function = shortcuts[i].function;
    sh->next = NULL;
    switch(shortcuts[i].modifier) {
    case MOD_NONE:
      sh->mod_mask = 0;
    break;
    case MOD_SHIFT:
      sh->mod_mask = XCB_MOD_MASK_SHIFT;
    break;
    case MOD_ALT:
      sh->mod_mask = XCB_MOD_MASK_1;
    break;
    case MOD_SUPER:
      sh->mod_mask = XCB_MOD_MASK_4;
    break;
    case MOD_CTRL:
      sh->mod_mask = XCB_MOD_MASK_CONTROL;
    break;
    }
  }

  if(shortcut_lookup[0] == NULL) {
    // find first used keycode
    for(size_t i=1;i<shortcut_lookup_l;i++) {
      if(shortcut_lookup[i] != NULL) {
        shortcut_lookup_offset += i;
        start = i;
        break;
      }
    }
    // find last keycode
    for(int i=shortcut_lookup_l-1; i>=0; i--) {
      if(shortcut_lookup[i] != NULL) {
        shortcut_lookup_l = i+1;
        break;
      }
    }
    shortcut_lookup_l -= start;
    // move keycodes to start
    memmove(shortcut_lookup, shortcut_lookup+start,
            shortcut_lookup_l*sizeof(void(*)(void)));
    // shrink array
    shortcut_lookup = realloc(shortcut_lookup,
                              shortcut_lookup_l*sizeof(void(*)(void)));
  }
}

uint32_t hex_to_uint(const char* str, size_t start, size_t end) {
  uint32_t mul = 1;
  uint32_t ret = 0;
  while(end --> start) {
    if(str[end] >= 'a') {
      ret += mul * (str[end] - 'a' + 10);
    } else if(str[end] >= 'A') {
      ret += mul * (str[end] - 'A' + 10);
    } else {
      ret += mul * (str[end] - '0');
    }
    mul *= 16;
  }
  return ret;
}

void hex_to_cairo_color(const char *str, double *cairo) {
  cairo[0] = hex_to_uint(str, 0, 2)/255.0;
  cairo[1] = hex_to_uint(str, 2, 4)/255.0;
  cairo[2] = hex_to_uint(str, 4, 6)/255.0;
}

#define color_parse(comp, def) \
  do { \
    comp.background = hex_to_uint(def ## _BACKGROUND, 0, 6); \
    hex_to_cairo_color(def ## _FOREGROUND, comp.foreground); \
  } while(0);

void parse_colors(void) {
  view.bar_settings.background = hex_to_uint(CONFIG_BAR_BACKGROUND, 0, 6);
  color_parse(view.bar_settings.mode_insert, CONFIG_BAR_MODE_INSERT);
  color_parse(view.bar_settings.mode_normal, CONFIG_BAR_MODE_NORMAL);

  color_parse(view.bar_settings.workspace_focused, CONFIG_BAR_WORKSPACE_FOCUSED);
  color_parse(view.bar_settings.workspace_unfocused, CONFIG_BAR_WORKSPACE_UNFOCUSED);

  color_parse(view.bar_settings.minimized_odd, CONFIG_BAR_MINIMIZED_ODD);
  color_parse(view.bar_settings.minimized_even, CONFIG_BAR_MINIMIZED_EVEN);

  color_parse(view.bar_settings.info, CONFIG_BAR_INFO);
  color_parse(view.bar_settings.info_highlighted, CONFIG_BAR_INFO_HIGHLIGHTED);

  color_parse(view.bar_settings.launcher_prompt, CONFIG_BAR_LAUNCHER_PROMPT);

  color_parse(view.bar_settings.launcher_hint, CONFIG_BAR_LAUNCHER_HINT);
  color_parse(view.bar_settings.launcher_hint_selected,
              CONFIG_BAR_LAUNCHER_HINT_SELECTED);
}

void config_parse(const xcb_get_keyboard_mapping_reply_t *kmapping) {
  convert_shortcuts(kmapping);
  view.spawn_order = malloc(sizeof((size_t[])CONFIG_SPAWN_ORDER));
  view.spawn_order_len = LENGTH((size_t[])CONFIG_SPAWN_ORDER);
  memcpy(view.spawn_order, (size_t[])CONFIG_SPAWN_ORDER,
         sizeof((size_t[])CONFIG_SPAWN_ORDER));

  view.bar_settings.height = CONFIG_BAR_HEIGHT;
  view.bar_settings.font = CONFIG_BAR_FONT;

  parse_colors();
}
