#include "bar.h"
#include "block.h"
#include "block_mode.h"
#include "block_workspace.h"
#include "block_minimized.h"
#include "block_info.h"
#include "launcher_container.h"
#include "launcher_prompt.h"
#include "launcher_hint.h"
#include "launcher_trie.h"
#include "bar_container.h"
#include <stdio.h>
#include <fontconfig/fontconfig.h>

//TODO: Possibly optimize searching
#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

#define CONF_COLOR(b, def) \
  to_block_settings(b, def ## _BACKGROUND, def ## _FOREGROUND)

static bool launcher_visible;
static bool bar_visible;
static xcb_connection_t *conn;
static const xcb_screen_t *screen;

static size_t (*focused_workspace)(void);
static bool (*workspace_empty)(size_t);
static const plist_t* (*minimized_list)(void);
static size_t minimized_name_offset;

static void bar_launcher_hint_refresh(void) {
  launcher_hint_regen(launcher_prompt_search, launcher_prompt_search_length);
  launcher_hint_update(block_next_x(&launcher_prompt_geometry));
}

void bar_launcher_show(void) {
  if(bar_visible) {
    launcher_container_show();
    launcher_visible = true;
    launcher_prompt_clear();
    xcb_ungrab_key(conn, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
    for(size_t i=0; i<bar_container_count; i++) {
      xcb_grab_key(conn, 1, bar_containers.launcher[i], XCB_MOD_MASK_ANY,
                   XCB_GRAB_ANY, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    }
    xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                        bar_containers.launcher[0], XCB_CURRENT_TIME);
    launcher_trie_unmark(launcher_trie_tree);
    launcher_trie_populate(&launcher_trie_tree);
    launcher_trie_cleanup(launcher_trie_tree);
    bar_launcher_hint_refresh();
  }
}

void bar_launcher_hide(void) {
  if(launcher_visible) {
    launcher_container_hide();
    launcher_visible = false;
    for(size_t i=0; i<bar_container_count; i++) {
      xcb_ungrab_key(conn, XCB_GRAB_ANY, bar_containers.launcher[i], XCB_MOD_MASK_ANY);
    }
  }
}

void bar_launcher_append(const char* buff, size_t len) {
  if(launcher_visible) {
    launcher_prompt_append(buff, len);
    bar_launcher_hint_refresh();
  }
}

void bar_launcher_erase(void) {
  if(launcher_visible) {
    launcher_prompt_erase();
    bar_launcher_hint_refresh();
  }
}

void bar_launcher_select_left(void) {
  if(launcher_visible) {
    launcher_hint_selected =
      (launcher_hint_selected+launcher_hint_count-1)%launcher_hint_count;
    launcher_hint_update(block_next_x(&launcher_prompt_geometry));
  }
}

void bar_launcher_select_right(void) {
  if(launcher_visible) {
    launcher_hint_selected = (launcher_hint_selected+1)%launcher_hint_count;
    launcher_hint_update(block_next_x(&launcher_prompt_geometry));
  }
}

char *bar_launcher_return(void) {
  if(launcher_visible) {
    bar_launcher_hide();
    return launcher_trie_hints[launcher_hint_selected];
  }
  return NULL;
}

bool bar_launcher_window(xcb_window_t window) {
  if(!launcher_visible) return false;
  for(size_t i=0; i<bar_container_count; i++) {
    if(bar_containers.launcher[i] == window)
      return true;
  }
  return false;
}

void bar_update_minimized(void) {
  if(bar_visible) {
    block_minimized_update(minimized_list(), minimized_name_offset,
                           block_next_x(block_workspace_geometry+MAX_WORKSPACE_BLOCKS-1),
                           block_info_offset_right);
  }
}

void bar_update_workspace(size_t n) {
  if(bar_visible) {
    size_t pos = block_next_x(block_workspace_geometry+(MAX_WORKSPACE_BLOCKS-1));
    bool update = (pos == block_minimized_geometry[0].x);
    block_workspace_update(focused_workspace(), workspace_empty,
                           block_next_x(&block_mode_geometry), n);
    if(update)
      bar_update_minimized();
  }
}

void bar_update_mode(MODE m) {
  if(bar_visible) {
    size_t pos = block_next_x(&block_mode_geometry);
    block_mode_update(m == MODE_NORMAL);
    if(block_next_x(&block_mode_geometry) != pos)
      bar_update_workspace(0);
  }
}

void bar_redraw(xcb_window_t window) {
  if(bar_visible) {
    size_t bar = bar_container_find(window);
    if(bar >= bar_container_count) {
      if(!launcher_visible) {
        if(block_mode_find_redraw(window)) return;
        if(block_workspace_find_redraw(window)) return;
        if(block_info_find_redraw(window)) return;
        if(block_minimized_find_redraw(window)) return;
      } else {
        if(launcher_prompt_find_redraw(window)) return;
        if(launcher_hint_find_redraw(window)) return;
      }
      return;
    }
  }
}

void bar_update_info_highlight(int n, int delay) {
  if(bar_visible)
    block_info_update_highlight(n, delay);
}

void bar_update_info(int n) {
  if(bar_visible)
    block_info_update(n);
}

void bar_visibility(bool st) {
  if(st) {
    for(size_t i=0; i<bar_container_count; i++) {
      xcb_map_window(conn, bar_containers.id[i]);
    }
  } else {
    bar_launcher_hide();
    for(size_t i=0; i<bar_container_count; i++) {
      xcb_unmap_window(conn, bar_containers.id[i]);
    }
  }
  bar_visible = st;
}

void bar_init(const bar_init_t *init) {
  conn = init->conn;
  screen = init->screen;
  PangoFontDescription *font;
  bar_containers_t containers;
  workspace_empty = init->workspace_empty;
  focused_workspace = init->focused_workspace;
  minimized_list = init->minimized_list;
  minimized_name_offset = init->minimized_name_offset;
  containers.x = malloc(init->bar_container_count*sizeof(uint16_t));
  containers.y = malloc(init->bar_container_count*sizeof(uint16_t));
  containers.w = malloc(init->bar_container_count*sizeof(uint16_t));
  containers.h = init->bar_containers[0].h;
  containers.background = block_background(init->bar_background, 0, 6);
  containers.padding = init->block_padding;
  containers.separator = init->block_separator;
  containers.launcher = NULL;
  for(size_t i=0; i<init->bar_container_count; i++) {
    containers.x[i] = init->bar_containers[i].x;
    containers.y[i] = init->bar_containers[i].y;
    containers.w[i] = init->bar_containers[i].w;
  }
  bar_container_init(conn, screen, containers, init->bar_container_count);
  bar_visible = true;
  block_init(conn, screen, init->visual_type);
  font = pango_font_description_from_string(init->bar_font);
  block_mode_init(font, &init->block_mode);
  block_workspace_init(font, &init->block_workspace);
  block_info_init(font, bar_update_minimized, &init->block_info,
                  conn);
  block_minimized_init(font, &init->block_minimized);
  launcher_container_init(conn, screen);
  launcher_prompt_init(font, &init->launcher_prompt);
  launcher_hint_init(font, &init->launcher_hint);
  pango_font_description_free(font);
  launcher_trie_populate(&launcher_trie_tree);
  bar_update_workspace(0);
  bar_update_minimized();
}

void bar_deinit(void) {
  launcher_trie_clear(launcher_trie_tree);
  launcher_hint_deinit();
  launcher_prompt_deinit();
  launcher_container_deinit();
  block_minimized_deinit();
  block_info_deinit();
  block_workspace_deinit();
  block_mode_deinit();
  block_deinit();
  bar_container_deinit();
  FcFini();
  pango_cairo_font_map_set_default(NULL);
  cairo_debug_reset_static_data();
}
