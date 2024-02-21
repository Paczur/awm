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

static bool launcher_visible;

//TODO: Possibly optimize searching
#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

#define CONF_COLOR(b, def) \
  to_block_settings(b, def ## _BACKGROUND, def ## _FOREGROUND)

static uint32_t hex_to_uint(const char* str, size_t start, size_t end) {
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

static void to_block_settings(block_settings_t *bs, char *back, char *fore) {
  bs->background = hex_to_uint(back, 0, 6);
  bs->foreground[0] = hex_to_uint(fore, 0, 2)/255.0;
  bs->foreground[1] = hex_to_uint(fore, 2, 4)/255.0;
  bs->foreground[2] = hex_to_uint(fore, 4, 6)/255.0;
}

void bar_launcher_show(void) {
  launcher_container_show();
  launcher_visible = true;
  launcher_prompt_clear();
  xcb_ungrab_key(conn, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
  for(size_t i=0; i<launcher_container_count; i++) {
    xcb_grab_key(conn, 1, launcher_containers.id[i], XCB_MOD_MASK_ANY,
                 XCB_GRAB_ANY, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
  }
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                      launcher_containers.id[0], XCB_CURRENT_TIME);
  launcher_trie_unmark(launcher_trie_tree);
  launcher_trie_populate(&launcher_trie_tree);
  launcher_trie_cleanup(launcher_trie_tree);
  launcher_hint_regen();
}

void bar_launcher_hide(void) {
  launcher_container_hide();
  launcher_visible = false;
  for(size_t i=0; i<launcher_container_count; i++) {
    xcb_ungrab_key(conn, XCB_GRAB_ANY, launcher_containers.id[i], XCB_MOD_MASK_ANY);
  }
  layout_focus_restore();
  normal_mode();
}

void bar_launcher_append(const xcb_key_press_event_t *event) {
  XKeyEvent keyev;
  char buff[10];
  size_t len;
  keyev = (XKeyEvent) {
    .type = KeyPress,
      .display = dpy,
      .keycode = event->detail,
      .state = event->state
  };
  len = Xutf8LookupString(xic, &keyev, buff, sizeof(buff), NULL, NULL);
  if(len > 0)
    launcher_prompt_append(buff, len);
  launcher_hint_regen();
}

void bar_launcher_erase(void) {
  launcher_prompt_erase();
  launcher_hint_regen();
}

void bar_launcher_select_left(void) {
  launcher_hint_selected =
    (launcher_hint_selected+launcher_hint_count-1)%launcher_hint_count;
  launcher_hint_update();
}

void bar_launcher_select_right(void) {
  launcher_hint_selected = (launcher_hint_selected+1)%launcher_hint_count;
  launcher_hint_update();
}

void bar_launcher_run(void) {
  bar_launcher_hide();
  sh(launcher_trie_hints[launcher_hint_selected]);
}

bool bar_launcher_window(xcb_window_t window) {
  if(!launcher_visible) return false;
  for(size_t i=0; i<launcher_container_count; i++) {
    if(launcher_containers.id[i] == window)
      return true;
  }
  return false;
}

void bar_update_minimized(void) {
  const window_list_t *wl = layout_get_minimized();
  block_minimized_update((const plist_t*)wl, offsetof(window_t, name),
                         block_next_x(block_workspace_geometry+MAX_WORKSPACE_BLOCKS-1),
                         block_info_offset_right);
}

void bar_update_workspace(void) {
  size_t pos = block_next_x(block_workspace_geometry+(MAX_WORKSPACE_BLOCKS-1));
  bool update = (pos == block_minimized_geometry[0].x);
  block_workspace_update(layout_get_focused_workspace(), layout_workspace_empty,
                         block_next_x(&block_mode_geometry));
  if(update)
    bar_update_minimized();
}

void bar_update_mode(void) {
  size_t pos = block_next_x(&block_mode_geometry);
  block_mode_update(mode_get() == MODE_NORMAL);
  if(block_next_x(&block_mode_geometry) != pos)
    bar_update_workspace();
}

void bar_update_info_highlight(int n, int delay) {
  block_info_update_highlight(n, delay);
}

void bar_update_info(int n) {
  block_info_update(n);
}

//TODO: more granular redrawing
void bar_redraw(void) {
  block_mode_redraw();
  block_workspace_redraw();
  block_info_redraw();
  block_minimized_redraw();
  launcher_prompt_redraw();
  launcher_hint_redraw();
}

void bar_init(const rect_t *bs, size_t count) {
  block_settings_t bsn;
  block_settings_t bsh;
  PangoFontDescription *font;
  bar_containers_t containers;
  containers.x = malloc(count*sizeof(uint16_t));
  containers.y = malloc(count*sizeof(uint16_t));
  containers.w = malloc(count*sizeof(uint16_t));
  containers.h = bs[0].h;
  containers.background = hex_to_uint(CONFIG_BAR_BACKGROUND, 0, 6);
  containers.padding = CONFIG_BAR_COMPONENT_PADDING;
  containers.separator = CONFIG_BAR_COMPONENT_SEPARATOR;
  for(size_t i=0; i<count; i++) {
    containers.x[i] = bs[i].x;
    containers.y[i] = bs[i].y;
    containers.w[i] = bs[i].w;
  }
  bar_container_init(conn, screen, containers, count);
  block_init(conn, screen, visual_type);
  font = pango_font_description_from_string(CONFIG_BAR_FONT);
  CONF_COLOR(&bsh, CONFIG_BAR_MODE_NORMAL);
  CONF_COLOR(&bsn, CONFIG_BAR_MODE_INSERT);
  block_mode_init(font, CONFIG_BAR_MODE_MIN_WIDTH, &bsh, &bsn);
  CONF_COLOR(&bsh, CONFIG_BAR_WORKSPACE_FOCUSED);
  CONF_COLOR(&bsn, CONFIG_BAR_WORKSPACE_UNFOCUSED);
  block_workspace_init(font, CONFIG_BAR_WORKSPACE_MIN_WIDTH, &bsh, &bsn);
  CONF_COLOR(&bsn, CONFIG_BAR_INFO);
  CONF_COLOR(&bsh, CONFIG_BAR_INFO_HIGHLIGHTED);
  block_info_init(font, CONFIG_BAR_INFO_MIN_WIDTH, &bsh, &bsn,
                  (block_info_data_t[])CONFIG_BAR_INFO_BLOCKS,
                  LENGTH((block_info_data_t[])CONFIG_BAR_INFO_BLOCKS),
                  conn);
  CONF_COLOR(&bsn, CONFIG_BAR_MINIMIZED_EVEN);
  CONF_COLOR(&bsh, CONFIG_BAR_MINIMIZED_ODD);
  block_minimized_init(font, CONFIG_BAR_MINIMIZED_MIN_WIDTH, &bsh, &bsn);
  launcher_container_init(conn, screen);
  CONF_COLOR(&bsn, CONFIG_BAR_LAUNCHER_PROMPT);
  launcher_prompt_init(font, CONFIG_BAR_LAUNCHER_PROMPT_MIN_WIDTH, &bsn);
  CONF_COLOR(&bsn, CONFIG_BAR_LAUNCHER_HINT);
  CONF_COLOR(&bsh, CONFIG_BAR_LAUNCHER_HINT_SELECTED);
  launcher_hint_init(font, CONFIG_BAR_LAUNCHER_HINT_MIN_WIDTH, &bsh, &bsn);
  launcher_trie_populate(&launcher_trie_tree);
  bar_update_mode();
  bar_update_workspace();
  bar_update_minimized();
}

void bar_deinit(void) {
  launcher_trie_clear(launcher_trie_tree);
  launcher_hint_deinit();
  launcher_prompt_deinit();
  launcher_container_deinit();
  block_minimized_deinit();
  block_workspace_deinit();
  block_mode_deinit();
  block_deinit();
  bar_container_deinit();
}
