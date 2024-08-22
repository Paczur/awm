#include "bar.h"

#include <fontconfig/fontconfig.h>
#include <stdio.h>

#include "bar_container.h"
#include "block.h"
#include "block_info.h"
#include "block_minimized.h"
#include "block_mode.h"
#include "block_workspace.h"
#include "launcher_container.h"
#include "launcher_hint.h"
#include "launcher_prompt.h"
#include "launcher_trie.h"

// TODO: Possibly optimize searching
#define LENGTH(x) (sizeof(x) / sizeof((x)[0]))

#define CONF_COLOR(b, def) \
  to_block_settings(b, def##_BACKGROUND, def##_FOREGROUND)

bool bar_launcher_visible;
static xcb_connection_t *conn;
static const xcb_screen_t *screen;
static const char *fontstr;

static size_t (*focused_workspace)(void);

static void bar_launcher_hint_refresh(void) {
  launcher_hint_regen(launcher_prompt_search, launcher_prompt_search_length);
  launcher_hint_update(block_next_x(&launcher_prompt_geometry));
}

size_t bar_get_containers(const bar_containers_t **cont) {
  *cont = &bar_containers;
  return bar_container_count;
}

void bar_launcher_focus_restore(void) {
  if(!bar_launcher_visible) return;
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                      bar_containers.launcher[0], XCB_CURRENT_TIME);
}

void bar_launcher_show(void) {
  size_t visible = -1;
  for(size_t i = 0; i < bar_container_count; i++) {
    if(bar_containers.visibility[i]) {
      visible = i;
      break;
    }
  }
  if(visible < bar_container_count) {
    launcher_container_show();
    bar_launcher_visible = true;
    launcher_prompt_clear();
    xcb_ungrab_key(conn, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
    for(size_t i = 0; i < bar_container_count; i++) {
      xcb_grab_key(conn, 1, bar_containers.launcher[i], XCB_MOD_MASK_ANY,
                   XCB_GRAB_ANY, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    }
    launcher_trie_unmark(launcher_trie_tree);
    launcher_trie_populate(&launcher_trie_tree);
    launcher_trie_cleanup(launcher_trie_tree);
    bar_launcher_hint_refresh();
  }
}

void bar_launcher_hide(void) {
  if(bar_launcher_visible) {
    launcher_container_hide();
    bar_launcher_visible = false;
    for(size_t i = 0; i < bar_container_count; i++) {
      xcb_ungrab_key(conn, XCB_GRAB_ANY, bar_containers.launcher[i],
                     XCB_MOD_MASK_ANY);
    }
  }
}

void bar_launcher_append(const char *buff, size_t len) {
  if(bar_launcher_visible) {
    launcher_prompt_append(buff, len);
    bar_launcher_hint_refresh();
  }
}

void bar_launcher_erase(void) {
  if(bar_launcher_visible) {
    launcher_prompt_erase();
    bar_launcher_hint_refresh();
  }
}

void bar_launcher_select_left(void) {
  if(bar_launcher_visible) {
    launcher_hint_selected =
      (launcher_hint_selected + launcher_hint_count - 1) % launcher_hint_count;
    launcher_hint_update(block_next_x(&launcher_prompt_geometry));
  }
}

void bar_launcher_select_right(void) {
  if(bar_launcher_visible) {
    launcher_hint_selected = (launcher_hint_selected + 1) % launcher_hint_count;
    launcher_hint_update(block_next_x(&launcher_prompt_geometry));
  }
}

char *bar_launcher_return(void) {
  if(bar_launcher_visible) {
    bar_launcher_hide();
    if(launcher_trie_hints[launcher_hint_selected][0] == 0)
      return launcher_prompt_search;
    return launcher_trie_hints[launcher_hint_selected];
  }
  return NULL;
}

puref bool bar_launcher_window(xcb_window_t window) {
  if(!bar_launcher_visible) return false;
  for(size_t i = 0; i < bar_container_count; i++) {
    if(bar_containers.launcher[i] == window) return true;
  }
  return false;
}

void bar_update_minimized(void) {
  block_minimized_update(
    block_next_x(block_workspace_geometry + MAX_WORKSPACE_BLOCKS - 1),
    block_info_offset_right);
}

void bar_update_workspace(size_t n) {
  size_t pos =
    block_next_x(block_workspace_geometry + (MAX_WORKSPACE_BLOCKS - 1));
  bool update = (pos == block_minimized_geometry[0].x);
  block_workspace_update(focused_workspace(),
                         block_next_x(&block_mode_geometry), n);
  if(update) bar_update_minimized();
}

void bar_update_mode(MODE m) {
  size_t pos = block_next_x(&block_mode_geometry);
  block_mode_set(m == MODE_NORMAL);
  if(block_next_x(&block_mode_geometry) == pos) bar_update_workspace(0);
}

void bar_redraw(xcb_window_t window) {
  size_t bar = bar_container_find(window);
  if(bar >= bar_container_count) {
    if(block_mode_find_redraw(window)) return;
    if(block_workspace_find_redraw(window)) return;
    if(block_info_find_redraw(window)) return;
    if(block_minimized_find_redraw(window)) return;
    if(bar_launcher_visible) {
      if(launcher_prompt_find_redraw(window)) return;
      if(launcher_hint_find_redraw(window)) return;
    }
    return;
  }
}

void bar_redraw_all(void) {
  for(size_t i = 0; i < bar_container_count; i++) {
    block_mode_redraw(i);
    block_workspace_redraw(i);
    block_info_redraw(i);
    block_minimized_redraw(i);
    if(bar_launcher_visible) {
      launcher_prompt_redraw(i);
      launcher_hint_redraw(i);
    }
  }
}

void bar_update_all(void) {
  block_mode_update();
  bar_update_workspace(1);
  block_info_update_all();
  bar_update_minimized();
}

void bar_update_info_highlight(int n, int delay) {
  block_info_update_highlight(n, delay);
}

void bar_update_info(int n) { block_info_update(n); }

void bar_visibility(size_t bar, bool st) {
  if(st == bar_containers.visibility[bar]) return;
  if(st) {
    xcb_map_window(conn, bar_containers.id[bar]);
    bar_update_minimized();
    bar_update_workspace(0);
  } else {
    bar_launcher_hide();
    xcb_unmap_window(conn, bar_containers.id[bar]);
  }
  bar_containers.visibility[bar] = st;
#define PRINT                         \
  OUT_CONTAINER(bar_containers, bar); \
  OUT(st);
  LOGF(BAR_TRACE);
#undef PRINT
}

void bar_color(size_t index) {
  launcher_container_color(index);
  bar_container_color(index);
  block_color(index);
  bar_update_all();
}

void bar_focus(xcb_window_t win) {
  if(bar_launcher_visible && !bar_launcher_window(win)) {
    bar_launcher_hide();
  }
}

void bar_count_update(const rect_t *rect, size_t count) {
  PangoFontDescription *font;
  bar_containers_t containers = bar_containers;
  size_t old = bar_container_count;
  if(count > bar_container_count) {
    containers.x = realloc(containers.x, count * sizeof(uint32_t));
    containers.y = realloc(containers.y, count * sizeof(uint32_t));
    containers.w = realloc(containers.w, count * sizeof(uint32_t));
    containers.visibility =
      realloc(containers.visibility, count * sizeof(uint32_t));
  }
  for(size_t i = 0; i < count; i++) {
    containers.x[i] = rect[i].x;
    containers.y[i] = rect[i].y;
    containers.w[i] = rect[i].w;
  }
  bar_container_update(containers, count);
  font = pango_font_description_from_string(fontstr);
  block_mode_count_update(font, old);
  block_info_count_update(font, old);
  block_minimized_count_update(font, old);
  block_workspace_count_update(font, old);
  launcher_container_count_update(old);
  launcher_prompt_count_update(font, old);
  launcher_hint_count_update(font, old);
  pango_font_description_free(font);
  bar_update_all();
}

void bar_init(const bar_init_t *init) {
  xcolor_t color;
  conn = init->conn;
  screen = init->screen;
  PangoFontDescription *font;
  fontstr = init->bar_font;
  bar_containers_t containers;
  focused_workspace = init->focused_workspace;
  containers.x = malloc(init->bar_container_count * sizeof(uint32_t));
  containers.y = malloc(init->bar_container_count * sizeof(uint32_t));
  containers.w = malloc(init->bar_container_count * sizeof(uint32_t));
  containers.h = init->bar_containers[0].h;
  containers.visibility = malloc(init->bar_container_count * sizeof(bool));
  memset(containers.visibility, 1, init->bar_container_count * sizeof(bool));
  block_background(init->bar_background, &color, 0, 6);
  memcpy(containers.background, color, sizeof(xcolor_t));
  containers.padding = init->block_padding;
  containers.separator = init->block_separator;
  containers.launcher = NULL;
  for(size_t i = 0; i < init->bar_container_count; i++) {
    containers.x[i] = init->bar_containers[i].x;
    containers.y[i] = init->bar_containers[i].y;
    containers.w[i] = init->bar_containers[i].w;
  }
  bar_container_init(conn, screen, containers, init->bar_container_count);
  block_init(conn, screen, init->visual_type);
  font = pango_font_description_from_string(init->bar_font);
  block_mode_init(font, &init->block_mode);
  block_workspace_init(font, &init->block_workspace);
  block_info_init(font, bar_update_minimized, &init->block_info, conn);
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
