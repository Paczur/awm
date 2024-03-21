#include "controller.h"
#include "system.h"
#include "bar/bar.h"
#include "layout/layout.h"
#include "hint/hint.h"
#include "event.h"
#include "shortcut.h"
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#define XK_LATIN1 //letters
#define XK_MISCELLANY //modifiers and special
#include <X11/keysymdef.h>
#include <X11/XF86keysym.h>
#include <string.h>

static MODE mode;
static MODE next_mode = MODE_INVALID;
#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))
#define MIN(x,y) ((x)<=(y)?(x):(y))

#define SETTINGS_INIT(x) \
{ x ## _BACKGROUND, x ## _FOREGROUND }

#define COMMON_INIT(x, n, h) \
{ x ## _MIN_WIDTH, SETTINGS_INIT(x ## _ ## n), SETTINGS_INIT(x ## _ ## h) }

#define COMMON_URGENT_INIT(x, n, h) \
{ x ## _MIN_WIDTH, SETTINGS_INIT(x ## _ ## n), SETTINGS_INIT(x ## _ ## h), \
  SETTINGS_INIT(x ## _URGENT)}

static void c_bar_update_minimized(void) {
  bar_update_minimized();
}
static void c_bar_update_workspace(size_t n) {
  bar_update_workspace(n);
}
static void c_bar_update_mode(void) {
  bar_update_mode(mode);
}
static void c_adopt_windows(void) {
  xcb_window_t *windows;
  size_t len = hint_get_saved_client_list(&windows);
  for(size_t i=0; i<len; i++) {
    layout_restore_window(windows[i], hint_get_saved_wm_desktop(windows[i]));
  }
  free(windows);
}

void c_shutdown(void) { event_stop(); }
void c_workspace_switch(size_t n) {
  bool prev;
  bool curr;
  if(n == layout_get_focused_workspace() || n > MAX_WORKSPACES) return;
  size_t min = MIN(n, layout_get_focused_workspace());
  prev = layout_workspace_fullscreen(layout_get_focused_workspace());
  curr = layout_workspace_fullscreen(n);
  layout_switch_workspace(n);
  hint_set_current_workspace(n);
  if(curr != prev)
    bar_visibility(!curr);
  c_bar_update_workspace(min);
}
void c_workspace_fullscreen(size_t n) {
  bool ret = layout_fullscreen(n);
  if(layout_get_focused_workspace() == n) {
    bar_visibility(!ret);
  }
}
void c_workspace_focused_fullscreen(void) {
  c_workspace_fullscreen(layout_get_focused_workspace());
}
void c_window_show(size_t n) {
  layout_show(n);
  c_bar_update_minimized();
}
void c_window_minimize(xcb_window_t window) {
  WINDOW_STATE state = layout_minimize(window);
  if(state == -1) return;
  if((size_t)state != layout_get_focused_workspace())
    c_bar_update_workspace(state);
  c_bar_update_minimized();
}
void c_window_destroy(xcb_window_t window, bool force) {
  if(force || !hint_delete_window(window))
    layout_destroy(window);
}
void c_window_focus(size_t n) { layout_focus_by_spawn(n); }
void c_window_focused_swap(size_t n) { layout_swap_focused_by_spawn(n); }
void c_window_focus_down(void) { layout_focus(layout_below()); }
void c_window_focus_up(void) { layout_focus(layout_above()); }
void c_window_focus_left(void) { layout_focus(layout_to_left()); }
void c_window_focus_right(void) { layout_focus(layout_to_right()); }
void c_window_focused_swap_down(void) { layout_swap_focused(layout_below()); }
void c_window_focused_swap_up(void) { layout_swap_focused(layout_above()); }
void c_window_focused_swap_left(void) { layout_swap_focused(layout_to_left()); }
void c_window_focused_swap_right(void) { layout_swap_focused(layout_to_right()); }
void c_window_focused_resize_w(int n) { layout_resize_w_focused(n); }
void c_window_focused_resize_h(int n) { layout_resize_h_focused(n); }
void c_window_focused_reset_size(void) { layout_reset_sizes_focused(); }
void c_run(const char* cmd) {
  const char rstr[] = " >/dev/null 2>&1";
  size_t len = strlen(cmd);
  char *rcmd = malloc(len + sizeof(rstr));
  memcpy(rcmd, cmd, len);
  memcpy(rcmd+len, rstr, sizeof(rstr));
  system_sh(rcmd);
  free(rcmd);
}
void c_window_focused_destroy(bool force) {
  c_window_destroy(layout_focused_xwin(), force);
}
void c_window_focused_minimize(void) {
  if(!layout_focused_minimize()) return;
  c_bar_update_minimized();
}
void c_launcher_show(void) { bar_launcher_show(); }
void c_launcher_cancel(void) {
  bar_launcher_hide();
  layout_focus_restore();
  c_mode_set(MODE_NORMAL);
}
void c_launcher_run(void) {
  c_run(bar_launcher_return());
  layout_focus_restore();
  c_mode_set(MODE_NORMAL);
}
void c_launcher_select_left(void) { bar_launcher_select_left(); }
void c_launcher_select_right(void) { bar_launcher_select_right(); }
void c_launcher_erase(void) { bar_launcher_erase(); }
void c_mode_delay(MODE m) { next_mode = m; }
void c_mode_force(void) { if(next_mode != MODE_INVALID) c_mode_set(next_mode); }
void c_mode_set(MODE m) {
  SHORTCUT_TYPE type = (m == MODE_INSERT)?
    SH_TYPE_INSERT_MODE:SH_TYPE_NORMAL_MODE;
  next_mode = MODE_INVALID;
  shortcut_enable(screen, type);
  mode = m;
  c_bar_update_mode();
}
void c_bar_block_update(size_t n) { bar_update_info(n); }
void c_bar_block_update_highlight(size_t n, int delay) {
  bar_update_info_highlight(n, delay);
}
void c_event_message(const xcb_generic_event_t *e) {
  const xcb_client_message_event_t *event = (const xcb_client_message_event_t*)e;
  if(event->type == hint_wm_change_state_atom() &&
     hint_is_iconic_state(event->data.data8[0])) {
    c_window_minimize(event->window);
  } else if(event->type == hint_close_window_atom()) {
    c_window_destroy(event->window, false);
  } else if(event->type == hint_frame_extents_atom()) {
    hint_set_frame_extents(event->window);
  }
}
void c_event_map(const xcb_generic_event_t *e) {
  const xcb_map_request_event_t *event = (const xcb_map_request_event_t*)e;
  xcb_atom_t *atoms;
  xcb_atom_t splash = hint_window_type_splash();
  size_t atom_length = hint_window_type(&atoms, event->window);
  for(size_t i=0; i<atom_length; i++) {
    if(atoms[i] == splash) {
      xcb_map_window(conn, event->window);
      free(atoms);
      return;
    }
  }
  if(!layout_event_map(event->window,
                       !hint_is_initial_state_normal(event->window))) {
    c_bar_update_minimized();
  }
  free(atoms);
}
void c_event_map_notify(const xcb_generic_event_t *e) {
  int mask = XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_ENTER_WINDOW |
    XCB_EVENT_MASK_PROPERTY_CHANGE;
  bool bar = false;
  const bar_containers_t *bar_containers;
  window_t *win;
  size_t bar_container_count;
  const xcb_map_notify_event_t *event = (const xcb_map_notify_event_t*)e;
  bar_container_count = bar_get_containers(&bar_containers);
  for(size_t i=0; i<bar_container_count; i++) {
    if(bar_containers->id[i] == event->window) {
      bar = true;
      break;
    }
  }
  if(!bar)
    layout_event_map_notify(event->window);
  xcb_change_window_attributes(conn, event->window, XCB_CW_EVENT_MASK, &mask);
  win = layout_window_find(event->window);
  if(win != NULL)
    layout_window_set_input(win, hint_input_state(event->window));
}
void c_event_key_press(const xcb_generic_event_t *e) {
  char buff[10];
  size_t len;
  const xcb_key_press_event_t *event = (const xcb_key_press_event_t*)e;
  if(bar_launcher_window(event->event)) {
    if(!shortcut_handle(event->detail, SH_TYPE_LAUNCHER, event->state)) {
      len = shortcut_utf8(event->detail, buff, 10);
      if(len > 0)
        bar_launcher_append(buff, len);
    }
    return;
  }
  if(mode == MODE_NORMAL) {
    shortcut_handle(event->detail, SH_TYPE_NORMAL_MODE, event->state);
  } else {
    shortcut_handle(event->detail, SH_TYPE_INSERT_MODE, event->state);
  }
}
void c_event_key_release(const xcb_generic_event_t *e) {
  const xcb_key_release_event_t *event = (const xcb_key_release_event_t*)e;
  if(mode == MODE_NORMAL) {
    shortcut_handle(event->detail, SH_TYPE_NORMAL_MODE_RELEASE, event->state);
  }
}
void c_event_create(const xcb_generic_event_t *e) {
  const xcb_create_notify_event_t *event = (const xcb_create_notify_event_t*)e;
  layout_event_create(event->window);
}
void c_event_destroy(const xcb_generic_event_t *e) {
  const xcb_destroy_notify_event_t *event = (const xcb_destroy_notify_event_t*)e;
  int t = layout_event_destroy(event->window);
  if(t == -1) {
    c_bar_update_minimized();
  } else if(t >= 0) {
    c_bar_update_workspace(t);
  }
}
void c_event_unmap(const xcb_generic_event_t *e) {
  const xcb_unmap_notify_event_t *event = (const xcb_unmap_notify_event_t*)e;
  layout_event_unmap(event->window);
}
void c_event_focus(const xcb_generic_event_t *e) {
  const xcb_focus_in_event_t *event = (const xcb_focus_in_event_t*)e;
  if(event->detail != XCB_NOTIFY_DETAIL_POINTER) {
    layout_event_focus(event->event);
    hint_set_focused_window(event->event);
  }
}
void c_event_expose(const xcb_generic_event_t *e) {
  const xcb_expose_event_t *event = (const xcb_expose_event_t*)e;
  bar_redraw(event->window);
}
void c_event_xkb(const xcb_generic_event_t *e) {
  shortcut_event_state((xcb_xkb_state_notify_event_t*)e);
}
void c_event_property(const xcb_generic_event_t *e) {
  window_t *win = NULL;
  const xcb_property_notify_event_t *event = (const xcb_property_notify_event_t*)e;
  if(hint_urgent_atom(event->atom)) {
    win = layout_window_find(event->window);
    bool state = hint_urgent_state(event->window, event->atom);
    if(layout_window_set_urgency(win, state)) {
      if(win->state == WINDOW_ICONIC) {
        c_bar_update_minimized();
      } else if(win->state != (int)layout_get_focused_workspace()) {
        c_bar_update_workspace(win->state);
      }
     }
  }
  if(hint_input_atom(event->atom)) {
    if(win == NULL)
      win = layout_window_find(event->window);
    layout_window_set_input(win, hint_input_state(event->window));
  }
}

static void convert_shortcuts(SHORTCUT_TYPE type,
                              config_shortcut_t *conf_shortcuts, size_t len) {
  xcb_mod_mask_t mod_mask;
  for(size_t i=0; i<len; i++) {
    mod_mask=0;
    if(conf_shortcuts[i].modifier & MOD_SHIFT)
      mod_mask |= XCB_MOD_MASK_SHIFT;
    if(conf_shortcuts[i].modifier & MOD_ALT)
      mod_mask |= XCB_MOD_MASK_1;
    if(conf_shortcuts[i].modifier & MOD_SUPER)
      mod_mask |= XCB_MOD_MASK_4;
    if(conf_shortcuts[i].modifier & MOD_CTRL)
      mod_mask |= XCB_MOD_MASK_CONTROL;
    shortcut_add(conf_shortcuts[i].keysym, type, mod_mask,
                 conf_shortcuts[i].function);
  }
}

static void c_init_bar(rect_t *t_rect, const rect_t *monitors,
                       size_t monitor_count) {
  for(size_t i=0; i<monitor_count; i++) {
    t_rect[i].x = monitors[i].x;
    t_rect[i].y = monitors[i].y;
    t_rect[i].w = monitors[i].w;
    t_rect[i].h = CONFIG_BAR_HEIGHT;
  }
  bar_init_t binit = (bar_init_t){
    conn, screen, visual_type, .bar_containers = t_rect,
      .bar_container_count = monitor_count, layout_get_focused_workspace,
      CONFIG_BAR_COMPONENT_PADDING,
      CONFIG_BAR_COMPONENT_SEPARATOR, CONFIG_BAR_BACKGROUND, CONFIG_BAR_FONT,
      COMMON_INIT(CONFIG_BAR_MODE, INSERT, NORMAL),
      { CONFIG_BAR_WORKSPACE_MIN_WIDTH,
        SETTINGS_INIT(CONFIG_BAR_WORKSPACE_UNFOCUSED),
        SETTINGS_INIT(CONFIG_BAR_WORKSPACE_FOCUSED),
        SETTINGS_INIT(CONFIG_BAR_WORKSPACE_URGENT),
        layout_workspace_empty, layout_workspace_urgent },
      { CONFIG_BAR_INFO_MIN_WIDTH,
        SETTINGS_INIT(CONFIG_BAR_INFO_NORMAL),
        SETTINGS_INIT(CONFIG_BAR_INFO_HIGHLIGHTED),
        SETTINGS_INIT(CONFIG_BAR_INFO_URGENT),
        (block_info_data_t[])CONFIG_BAR_INFO_BLOCKS,
        LENGTH((block_info_data_t[])CONFIG_BAR_INFO_BLOCKS), system_sh_out },
      { CONFIG_BAR_MINIMIZED_MIN_WIDTH,
        SETTINGS_INIT(CONFIG_BAR_MINIMIZED_EVEN),
        SETTINGS_INIT(CONFIG_BAR_MINIMIZED_ODD),
        SETTINGS_INIT(CONFIG_BAR_MINIMIZED_URGENT),
        (const plist_t *const*)layout_get_minimizedp(),
        layout_get_window_lock(), offsetof(window_t, name),
        offsetof(window_t, urgent) },
      { CONFIG_BAR_LAUNCHER_PROMPT_MIN_WIDTH,
        SETTINGS_INIT(CONFIG_BAR_LAUNCHER_PROMPT) },
      COMMON_INIT(CONFIG_BAR_LAUNCHER_HINT, NORMAL, SELECTED)
  };
  bar_init(&binit);
}

static void c_init_layout(rect_t *t_rect, const rect_t *monitors,
                          size_t monitor_count) {
  for(size_t i=0; i<monitor_count; i++) {
    t_rect[i].x = monitors[i].x;
    t_rect[i].y = monitors[i].y + CONFIG_BAR_HEIGHT;
    t_rect[i].w = monitors[i].w;
    t_rect[i].h = monitors[i].h - CONFIG_BAR_HEIGHT;
  }
  layout_init_t linit = (layout_init_t) {
    conn, screen, hint_window_class, .workareas = t_rect,
    .window_state_changed = hint_update_state,
    .workareas_fullscreen = monitors, .workarea_count = monitor_count,
      .name_replacements = (const char *const [][2])CONFIG_BAR_MINIMIZED_NAME_REPLACEMENTS,
      .name_replacements_length = LENGTH((char*[][2])CONFIG_BAR_MINIMIZED_NAME_REPLACEMENTS),
      .gaps = CONFIG_GAPS, .spawn_order = (const size_t[])CONFIG_SPAWN_ORDER,
      .spawn_order_length = LENGTH((size_t[])CONFIG_SPAWN_ORDER)
  };
  layout_init(&linit);
}

static void c_init_shortcut(void) {
  config_shortcut_t insert_shortcuts[] = CONFIG_SHORTCUTS_INSERT_MODE;
  config_shortcut_t normal_shortcuts[] = CONFIG_SHORTCUTS_NORMAL_MODE;
  config_shortcut_t launcher_shortcuts[] = CONFIG_SHORTCUTS_LAUNCHER;
  config_shortcut_t normal_release_shortcuts[] = CONFIG_SHORTCUTS_NORMAL_MODE_RELEASE;
  shortcut_init(conn);
  convert_shortcuts(SH_TYPE_INSERT_MODE, insert_shortcuts, LENGTH(insert_shortcuts));
  convert_shortcuts(SH_TYPE_NORMAL_MODE_RELEASE, normal_release_shortcuts,
                    LENGTH(normal_release_shortcuts));
  convert_shortcuts(SH_TYPE_NORMAL_MODE, normal_shortcuts,
                    LENGTH(normal_shortcuts));
  convert_shortcuts(SH_TYPE_LAUNCHER, launcher_shortcuts,
                    LENGTH(launcher_shortcuts));
}

void c_loop(void) { event_run(conn); }

void c_init(void) {
  rect_t *monitors;
  size_t monitor_count;
  size_t bar_count;
  const bar_containers_t *bars;
  rect_t *t_rect;

  system_init();
  hint_init(&(hint_init_t){conn, screen,
            {layout_get_workspaces(NULL), layout_workspace_names()}});
  c_init_shortcut();
  system_monitors(&monitors, &monitor_count);
  t_rect = malloc(monitor_count*sizeof(rect_t));
  c_init_layout(t_rect, monitors, monitor_count);
  c_init_bar(t_rect, monitors, monitor_count);
  free(monitors);
  free(t_rect);
  bar_count = bar_get_containers(&bars);
  for(size_t i=0; i<bar_count; i++) {
    hint_set_window_hints(bars->id[i]);
  }
  c_workspace_switch(hint_get_saved_current_desktop());
  c_adopt_windows();
  layout_event_focus(hint_get_saved_focused_window());

  c_mode_set(MODE_NORMAL);
  event_listener_add(XCB_MAP_REQUEST, c_event_map);
  event_listener_add(XCB_MAP_NOTIFY, c_event_map_notify);
  event_listener_add(XCB_KEY_PRESS, c_event_key_press);
  event_listener_add(XCB_KEY_RELEASE, c_event_key_release);
  event_listener_add(XCB_CREATE_NOTIFY, c_event_create);
  event_listener_add(XCB_DESTROY_NOTIFY, c_event_destroy);
  event_listener_add(XCB_UNMAP_NOTIFY, c_event_unmap);
  event_listener_add(XCB_FOCUS_IN, c_event_focus);
  event_listener_add(XCB_EXPOSE, c_event_expose);
  event_listener_add(XCB_CLIENT_MESSAGE, c_event_message);
  event_listener_add(XCB_PROPERTY_NOTIFY, c_event_property);
  event_listener_add(system_xkb(), c_event_xkb);
  xcb_flush(conn);
}

void c_deinit(void) {
  hint_deinit();
  bar_deinit();
  layout_deinit();
  shortcut_deinit();
  system_deinit();
}
