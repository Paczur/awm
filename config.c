#include "config.h"
#include "window.h"
#include "user_config.h"
#include "global.h"
#include "bar.h"
#include <stdlib.h> //calloc
#include <string.h> //memmove
#include <stdio.h> //printf

#define X(pos) ((pos)%2)
#define Y(pos) ((pos)%4/2)
#define COMB(x, y) ((x)+(y)*2)

#define fwindow_impl(n) \
  void focus_window_ ## n (void) { \
    focus_window_n(n); \
  }
#define swindow_impl(n) \
  void swap_window_ ## n (void) { \
    swap_windows(n, view.workspaces[view.focus].focus); \
  }

xcb_keycode_t keysym_to_keycode(xcb_keysym_t keysym) {
  for(int i=setup->min_keycode; i<setup->max_keycode; i++) {
    if(keysyms[(i-setup->min_keycode) * kmapping->keysyms_per_keycode] == keysym)
      return i;
  }
  return -1;
}

void convert_shortcuts(void) {
  int start = 0;
  xcb_keycode_t index;
  internal_shortcut_t *sh;

  normal_code = keysym_to_keycode(normal_shortcut);

  shortcut_lookup_l = setup->max_keycode-setup->min_keycode;
  shortcut_lookup_offset = setup->min_keycode;
  shortcut_lookup = calloc(shortcut_lookup_l, sizeof(internal_shortcut_t*));


  // get keycodes from keysyms
  for(size_t i=0; i<LENGTH(shortcuts); i++) {
    index = keysym_to_keycode(shortcuts[i].keysym)-shortcut_lookup_offset;
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

uint32_t hex_to_uint(char* str, size_t start, size_t end) {
  uint32_t mul = 1;
  uint32_t ret = 0;
  while(end --> start) {
    if(str[end] > '9') {
      ret += mul * (str[end] - 'A' + 10);
    } else {
      ret += mul * (str[end] - '0');
    }
    mul *= 16;
  }
  return ret;
}

void hex_to_cairo_color(char *str, double *cairo) {
  cairo[0] = hex_to_uint(str, 0, 2)/255.0;
  cairo[1] = hex_to_uint(str, 2, 4)/255.0;
  cairo[2] = hex_to_uint(str, 4, 6)/255.0;
}

void config_parse(void) {
  convert_shortcuts();
  view.bar_settings.height = bar_height;
  view.bar_settings.font = bar_font;
  view.bar_settings.component_padding = bar_component_padding;
  view.bar_settings.component_separator = bar_component_separator;
  view.bar_settings.mode_min_width = bar_mode_min_width;
  view.bar_settings.workspace_min_width = bar_workspace_min_width;

  view.bar_settings.background = hex_to_uint(bar_background, 0, 6);
  view.bar_settings.mode_insert.background =
    hex_to_uint(bar_mode_insert_background, 0, 6);
  view.bar_settings.mode_normal.background =
    hex_to_uint(bar_mode_normal_background, 0, 6);
  hex_to_cairo_color(bar_mode_insert_foreground,
                     view.bar_settings.mode_insert.foreground);
  hex_to_cairo_color(bar_mode_normal_foreground,
                     view.bar_settings.mode_normal.foreground);

  view.bar_settings.workspace_focused.background =
    hex_to_uint(bar_workspace_focused_background, 0, 6);
  view.bar_settings.workspace_unfocused.background =
    hex_to_uint(bar_workspace_unfocused_background, 0, 6);
  hex_to_cairo_color(bar_workspace_focused_foreground,
                     view.bar_settings.workspace_focused.foreground);
  hex_to_cairo_color(bar_workspace_unfocused_foreground,
                     view.bar_settings.workspace_unfocused.foreground);
}

void focus_window_down(void) {
  focus_window_n(window_below());
}

void focus_window_up(void) {
  focus_window_n(window_above());
}

void focus_window_left(void) {
  focus_window_n(window_to_left());
}

void focus_window_right(void) {
  focus_window_n(window_to_right());
}

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

void enlarge_width(void) {
  resize_w(view.workspaces[view.focus].focus/4, 10);
}

void enlarge_height(void) {
  resize_h(view.workspaces[view.focus].focus/4, 10);
}

void shrink_width(void) {
  resize_w(view.workspaces[view.focus].focus/4, -10);
}

void shrink_height(void) {
  resize_h(view.workspaces[view.focus].focus/4, -10);
}

fwindow_impl(0)
fwindow_impl(1)
fwindow_impl(2)
fwindow_impl(3)
fwindow_impl(4)
fwindow_impl(5)
fwindow_impl(6)
fwindow_impl(7)
fwindow_impl(8)
fwindow_impl(9)

swindow_impl(0)
swindow_impl(1)
swindow_impl(2)
swindow_impl(3)
swindow_impl(4)
swindow_impl(5)
swindow_impl(6)
swindow_impl(7)
swindow_impl(8)
swindow_impl(9)

void terminal(void) {
  sh("mlterm");
}

void normal_mode(void) {
  internal_shortcut_t *sh;
  mode = MODE_NORMAL;
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

void destroy_current_window(void) {
  destroy_n(view.workspaces[view.focus].focus);
}

void librewolf(void) {
  sh("lb");
}

