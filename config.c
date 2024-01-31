#include "config.h"
#include "window.h"
#include "user_config.h"
#include "global.h"
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

xcb_keycode_t keysym_to_keycode(xcb_keysym_t keysym) {
  for(int i=setup->min_keycode; i<setup->max_keycode; i++) {
    if(keysyms[(i-setup->min_keycode) * kmapping->keysyms_per_keycode] == keysym)
      return i;
  }
  return -1;
}

void convert_shortcuts(void) {
  int start = 0;

  normal_code = keysym_to_keycode(normal_shortcut);

  shortcut_lookup_l = setup->max_keycode-setup->min_keycode;
  shortcut_lookup_offset = setup->min_keycode;
  shortcut_lookup = calloc(shortcut_lookup_l, sizeof(void(*)(void)));


  // get keycodes from keysyms
  for(size_t i=0; i<LENGTH(shortcuts); i++) {
    shortcut_lookup[keysym_to_keycode(shortcuts[i].keysym)-shortcut_lookup_offset] =
      shortcuts[i].function;
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

uint32_t hex_to_uint(char* str, size_t len) {
  uint32_t mul = 1;
  uint32_t ret = 0;
  while(len --> 0) {
    if(str[len] > '9') {
      ret += mul * (str[len] - 'A' + 10);
    } else {
      ret += mul * (str[len] - '0');
    }
    mul *= 16;
  }
  return ret;
}

void config_parse(void) {
  convert_shortcuts();
  bar.height = bar_height;
  bar.color = hex_to_uint(bar_color, 6);
}

void focus_window_down(void) {
  size_t t = view.workspaces[view.focus].focus;
  focus_window_n(t/4*4+COMB(X(t), !Y(t)));
}

void focus_window_up(void) {
  focus_window_down();
}

void focus_window_left(void) {
  workspace_t *workspace = view.workspaces+view.focus;
  window_t *next = workspace->grid[workspace->focus].window;
  window_t *prev = workspace->grid[workspace->focus].window;
  size_t t = workspace->focus;
  while(prev == next) {
    if(t == 0 || t == 2) {
      t = COMB(0, Y(t)) + view.monitor_count*2-1;
    } else {
      t -= X(t) == 0 ? 3 : 1;
    }
    next = workspace->grid[t].window;
  }
  focus_window_n(t);
}

void focus_window_right(void) {
  workspace_t *workspace = view.workspaces+view.focus;
  window_t *next = workspace->grid[workspace->focus].window;
  window_t *prev = workspace->grid[workspace->focus].window;
  size_t t = workspace->focus;
  while(prev == next) {
    if(t == view.monitor_count*4-1 || t == view.monitor_count*4-3) {
      t = COMB(0, Y(t));
    } else {
      t += X(t) == 1 ? 3 : 1;
    }
    next = workspace->grid[t].window;
  }
  focus_window_n(t);
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

void terminal(void) {
  sh("mlterm");
}

void normal_mode(void) {
  mode = MODE_NORMAL;
  for(size_t i=0; i<shortcut_lookup_l; i++) {
    if(shortcut_lookup[i] != NULL)
      xcb_grab_key(conn, 1, screen->root, 0, i+shortcut_lookup_offset,
                   XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
  }
}

void insert_mode(void) {
  mode = MODE_INSERT;
  xcb_ungrab_key(conn, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
  xcb_grab_key(conn, 1, screen->root, 0, normal_code,
               XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
}

void destroy_current_window(void) {
  destroy_n(view.workspaces[view.focus].focus);
}

void librewolf(void) {
  sh("lb");
}

