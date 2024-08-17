#include "controller.h"

#include <stdio.h>
#include <stdlib.h>

#include "bar/bar.h"
#include "config.h"
#include "event.h"
#include "hint/hint.h"
#include "layout/layout.h"
#include "shortcut.h"
#include "system/system.h"
#define XK_LATIN1      // letters
#define XK_MISCELLANY  // modifiers and special
#include <X11/XF86keysym.h>
#include <X11/keysymdef.h>
#include <string.h>

static bool run;
static MODE mode;
static MODE next_mode = MODE_INVALID;
static xcolor_t border_normal;
static xcolor_t border_urgent;
size_t c_wm_color_current = 0;
#define LENGTH(x) (sizeof(x) / sizeof((x)[0]))
#define MIN(x, y) ((x) <= (y) ? (x) : (y))

#define SETTINGS_INIT(x) {x##_BACKGROUND, x##_FOREGROUND}

#define COMMON_INIT(x, n, h) \
  {x##_MIN_WIDTH, SETTINGS_INIT(x##_##n), SETTINGS_INIT(x##_##h)}

#define COMMON_URGENT_INIT(x, n, h)                               \
  {x##_MIN_WIDTH, SETTINGS_INIT(x##_##n), SETTINGS_INIT(x##_##h), \
   SETTINGS_INIT(x##_URGENT)}

static uint32_t c_hex2xcolor(const char *hex) {
  uint32_t mul = 1;
  uint32_t ret = 0;
  size_t end = 6;
  while(end-- > 0) {
    if(hex[end] >= 'a') {
      ret += mul * (hex[end] - 'a' + 10);
    } else if(hex[end] >= 'A') {
      ret += mul * (hex[end] - 'A' + 10);
    } else {
      ret += mul * (hex[end] - '0');
    }
    mul *= 16;
  }
  return ret;
}

static void c_focus_restore(void) {
  if(!bar_launcher_visible) {
    layout_focus_restore();
  } else {
    layout_focus_lose();
    bar_launcher_focus_restore();
  }
}

static void c_window_init(xcb_window_t window) {
  const window_t *win;
  int values[2] = {border_normal[c_wm_color_current],
                   XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_ENTER_WINDOW |
                     XCB_EVENT_MASK_PROPERTY_CHANGE};
  xcb_change_window_attributes(conn, window,
                               XCB_CW_EVENT_MASK | XCB_CW_BORDER_PIXEL, values);
  xcb_grab_button(conn, 1, window, XCB_EVENT_MASK_BUTTON_PRESS,
                  XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE,
                  XCB_BUTTON_INDEX_ANY, XCB_MOD_MASK_ANY);
  win = layout_xwin2win(window);
  if(win == NULL) {
    win = layout_create(window);
  }
  layout_input_set(layout_xwin2win(window), hint_window_input(window));
  c_focus_restore();
}

static void c_bar_update_workspace(size_t n) { bar_update_workspace(n); }

static void c_bar_update_minimized(void) {
  bar_update_minimized();
  if(!CONFIG_WORKSPACE_NUMBERS_ONLY) {
    c_bar_update_workspace(0);
  }
}

static void c_bar_update_mode(void) { bar_update_mode(mode); }

static void c_windows_adopt(void) {
  xcb_window_t *windows;
  size_t len = hint_saved_windows(&windows);
  for(size_t i = 0; i < len; i++) {
    layout_restore(windows[i], hint_saved_window_workspace(windows[i]));
    c_window_init(windows[i]);
  }
#define PRINT OUT_ARR(windows, len);
  LOGF(TRACE);
#undef PRINT
  free(windows);
}

static void c_sigterm_action(int a) {
  (void)a;
  c_wm_shutdown();
}

void c_focus_xwindow(xcb_window_t xwin) {
  window_t *win = layout_xwin2win(xwin);
  if(win) {
    layout_focus(win);
  }
}

void c_wm_shutdown(void) {
  run = false;
  LOGE(DEBUG, "Shutdown initiated");
}

void c_wm_color(size_t index) {
  c_wm_color_current = index;
  bar_color(index);
}

void c_workspace_switch(size_t n) {
  bool old_maximized = false;
  size_t workarea_count;
  bool new_maximized = false;
  size_t focused_workspace = layout_workspace_focused();
  if(n == focused_workspace || n > MAX_WORKSPACES) return;
  size_t refresh = MIN(n, focused_workspace);
  workarea_count = layout_workareas(NULL);
  for(size_t i = 0; i < workarea_count; i++) {
    old_maximized = layout_workspace_area_isfullscreen(focused_workspace, i);
    new_maximized = layout_workspace_area_isfullscreen(n, i);
    if(new_maximized != old_maximized) bar_visibility(i, !new_maximized);
  }
  layout_workspace_switch(n);
  hint_workspace_focused_set(n);
  c_bar_update_workspace(refresh);
#define PRINT \
  OUT(n);     \
  OUT(refresh);
  LOGF(TRACE);
#undef PRINT
}

void c_workspace_fullscreen(size_t n, size_t m) {
  bool old_state = layout_workspace_area_isfullscreen(n, m);
  size_t focused_workspace = layout_workspace_focused();
  bool new_state = layout_workspace_area_fullscreen_toggle(n, m);
  if(focused_workspace == n && new_state != old_state) {
    bar_visibility(m, !new_state);
  }

#define PRINT             \
  OUT(n);                 \
  OUT(focused_workspace); \
  OUT(old_state);         \
  OUT(new_state);
  LOGF(TRACE);
#undef PRINT
}

void c_workspace_focused_fullscreen(void) {
  size_t focused_workspace = layout_workspace_focused();
  c_workspace_fullscreen(focused_workspace, layout_area_focused());
}

xcb_window_t c_window_focused(void) {
  return layout_win2xwin(layout_focused());
}

void c_window_show(size_t n) {
  layout_show(n);
  c_bar_update_minimized();
#define PRINT OUT(n);
  LOGF(TRACE);
#undef PRINT
}

bool c_window_minimize(xcb_window_t xwin) {
  if(xwin == (xcb_window_t)-1) return false;
  window_t *window = layout_xwin2win(xwin);
  if(window == NULL) return false;
  layout_minimize(window);
#define PRINT OUT_WINDOW(window);
  LOGF(TRACE);
#undef PRINT
  return true;
}

void c_window_destroy(xcb_window_t window, bool force) {
  if(force || !hint_window_delete(window)) layout_destroy(window);
#define PRINT  \
  OUT(window); \
  OUT(force);
  LOGF(TRACE);
#undef PRINT
}

void c_window_focus(size_t n) {
  if(layout_focus(layout_spawn2win(n))) {
#define PRINT OUT(n);
    LOGF(TRACE);
#undef PRINT
  }
}

void c_window_focused_swap(size_t n) {
  if(layout_swap(layout_focused(), layout_spawn2win(n))) {
#define PRINT OUT(n);
    LOGF(TRACE);
#undef PRINT
  }
}

void c_window_focus_down(void) {
  if(layout_focus(layout_below())) {
    LOGFE(TRACE);
  }
}

void c_window_focus_up(void) {
  if(layout_focus(layout_above())) {
    LOGFE(TRACE);
  }
}

void c_window_focus_left(void) {
  if(layout_focus(layout_to_left())) {
    LOGFE(TRACE);
  }
}

void c_window_focus_right(void) {
  if(layout_focus(layout_to_right())) {
    LOGFE(TRACE);
  }
}

void c_window_focused_swap_down(void) {
  if(layout_swap(layout_focused(), layout_below())) {
    LOGFE(TRACE);
  }
}

void c_window_focused_swap_up(void) {
  if(layout_swap(layout_focused(), layout_above())) {
    LOGFE(TRACE);
  }
}

void c_window_focused_swap_left(void) {
  if(layout_swap(layout_focused(), layout_to_left())) {
    LOGFE(TRACE);
  }
}

void c_window_focused_swap_right(void) {
  if(layout_swap(layout_focused(), layout_to_right())) {
    LOGFE(TRACE);
  }
}

void c_window_focused_resize_w(int n) {
  layout_resize_w(layout_focused(), n);
#define PRINT OUT(n);
  LOGF(TRACE);
#undef PRINT
}

void c_window_focused_resize_h(int n) {
  layout_resize_h(layout_focused(), n);
#define PRINT OUT(n);
  LOGF(TRACE);
#undef PRINT
}

void c_window_focused_reset_size(void) {
  layout_reset_sizes(layout_focused());
  LOGFE(TRACE);
}

void c_run(const char *cmd) {
  const char rstr[] = " >/dev/null 2>&1";
  size_t len = strlen(cmd);
  char *rcmd = malloc(len + sizeof(rstr));
  memcpy(rcmd, cmd, len);
  memcpy(rcmd + len, rstr, sizeof(rstr));
  system_sh(rcmd);
#define PRINT OUT(rcmd);
  LOGF(TRACE);
#undef PRINT
  free(rcmd);
}

void c_window_focused_destroy(bool force) {
  if(layout_focused())
    c_window_destroy(layout_win2xwin(layout_focused()), force);
}

void c_window_focused_minimize(void) {
  if(!layout_minimize(layout_focused())) return;
  c_bar_update_minimized();
  LOGFE(TRACE);
}

void c_launcher_show(void) {
  bar_launcher_show();
  c_focus_restore();
  LOGFE(TRACE);
}

void c_launcher_cancel(void) {
  if(bar_launcher_visible) bar_launcher_hide();
  c_focus_restore();
  c_mode_set(MODE_NORMAL);
  LOGFE(TRACE);
}

void c_launcher_run(void) {
  c_run(bar_launcher_return());
  c_mode_set(MODE_NORMAL);
  c_focus_restore();
  LOGFE(TRACE);
}

void c_launcher_select_left(void) {
  bar_launcher_select_left();
  LOGFE(TRACE);
}

void c_launcher_select_right(void) {
  bar_launcher_select_right();
  LOGFE(TRACE);
}

void c_launcher_erase(void) {
  bar_launcher_erase();
  LOGFE(TRACE);
}

void c_mode_delay(MODE m) {
  next_mode = m;
  LOGFE(TRACE);
}

void c_mode_force(void) {
  MODE mode = next_mode;
  if(mode != MODE_INVALID) c_mode_set(next_mode);
#define PRINT OUT_MODE(mode);
  LOGF(TRACE);
#undef PRINT
}

void c_mode_set(MODE new_mode) {
  MODE old_mode = mode;
  SHORTCUT_TYPE type =
    (new_mode == MODE_INSERT) ? SH_TYPE_INSERT : SH_TYPE_NORMAL;
  next_mode = MODE_INVALID;
  shortcut_enable(screen, type);
  mode = new_mode;
  c_bar_update_mode();
#define PRINT         \
  OUT_MODE(old_mode); \
  OUT_MODE(new_mode);
  LOGF(TRACE);
#undef PRINT
}

void c_bar_block_update(size_t n) {
  bar_update_info(n);
#define PRINT OUT(n);
  LOGF(TRACE);
#undef PRINT
}

void c_bar_block_update_highlight(size_t n, int delay) {
  bar_update_info_highlight(n, delay);
#define PRINT \
  OUT(n);     \
  OUT(delay);
  LOGF(TRACE);
#undef PRINT
}

void c_event_message(const xcb_generic_event_t *e) {
  const xcb_client_message_event_t *event =
    (const xcb_client_message_event_t *)e;
  if(hint_atom_wm_change_state(event->type) &&
     hint_state_iconic(event->data.data8[0])) {
    c_window_minimize(event->window);
#define PRINT OUT(event->type);
    LOG(TRACE, "event: client_message: change state");
#undef PRINT
  } else if(hint_atom_close_window(event->type)) {
    c_window_destroy(event->window, false);
#define PRINT       \
  OUT(event->type); \
  OUT(event->window);
    LOG(TRACE, "event: client_message: close window");
  } else if(hint_atom_frame_extents(event->type)) {
    hint_frame_extents_set(event->window);
    LOG(TRACE, "event: client_message: get frame extents");
#undef PRINT
  }
}

void c_event_map(const xcb_generic_event_t *e) {
  const xcb_map_request_event_t *event = (const xcb_map_request_event_t *)e;
  xcb_atom_t *atoms;
  rect_t rect;
  size_t atom_length = hint_atom_window_type(&atoms, event->window);
  for(size_t i = 0; i < atom_length; i++) {
    if(hint_atom_window_type_splash(atoms[i]) ||
       hint_atom_window_type_utility(atoms[i]) ||
       hint_atom_window_type_notification(atoms[i])) {
      hint_window_rect_set(event->window, &rect);
      if(rect.x != (uint32_t)-1 && rect.y != (uint32_t)-1 && rect.w != 0 &&
         rect.h != 0 && rect.w != (uint32_t)-1 && rect.h != (uint32_t)-1) {
        xcb_configure_window(conn, event->window,
                             XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                               XCB_CONFIG_WINDOW_WIDTH |
                               XCB_CONFIG_WINDOW_HEIGHT,
                             &rect);
      }
      xcb_map_window(conn, event->window);
      free(atoms);
#define PRINT         \
  OUT(event->window); \
  OUT_RECT(rect);
      LOG(TRACE, "event: map_request(splash/utility window)");
#undef PRINT
      return;
    }
  }
  c_window_init(event->window);
  if(!layout_event_map(event->window,
                       !hint_initial_state_normal(event->window))) {
    c_bar_update_minimized();
  } else if(!CONFIG_WORKSPACE_NUMBERS_ONLY) {
    bar_update_workspace(layout_workspace_focused());
  }
  c_focus_restore();
#define PRINT OUT(event->window)
  LOG(TRACE, "event: map_request");
#undef PRINT
  free(atoms);
}

void c_event_map_notify(const xcb_generic_event_t *e) {
  bool bar = false;
  const bar_containers_t *bar_containers;
  window_t *win;
  size_t bar_container_count;
  const xcb_map_notify_event_t *event = (const xcb_map_notify_event_t *)e;
  bar_container_count = bar_get_containers(&bar_containers);
  for(size_t i = 0; i < bar_container_count; i++) {
    if(bar_containers->id[i] == event->window) {
      bar = true;
      break;
    }
  }
  if(!bar) {
    win = layout_xwin2win(event->window);
    layout_input_set(win, hint_window_input(event->window));
    layout_event_map_notify(event->window);
    c_focus_restore();
    if(TRACE && win != NULL) {
#define PRINT OUT_WINDOW(win);
      LOG(TRACE, "event: map_notify");
#undef PRINT
      return;
    }
  }
#define PRINT OUT(event->window)
  LOG(TRACE, "event: map_notify");
#undef PRINT
}

void c_event_key_press(const xcb_generic_event_t *e) {
  const xcb_key_press_event_t *event = (const xcb_key_press_event_t *)e;
  char buff[10];
  size_t len;
  if(bar_launcher_window(event->event)) {
    if(!shortcut_handle(event, SH_TYPE_LAUNCHER)) {
      len = shortcut_utf8(event->detail, buff, 10);
      if(len > 0) bar_launcher_append(buff, len);
    }
#define PRINT         \
  OUT(event->detail); \
  OUT(event->state);  \
  OUT(event->event);
    LOG(TRACE, "event: key_press(launcher)");
#undef PRINT
    return;
  }
  if(mode == MODE_NORMAL) {
    shortcut_handle(event, SH_TYPE_NORMAL);
  } else {
    shortcut_handle(event, SH_TYPE_INSERT);
  }
#define PRINT         \
  OUT(event->detail); \
  OUT(event->state);  \
  OUT_MODE(mode);
  LOG(TRACE, "event: key_press");
#undef PRINT
}

void c_event_key_release(const xcb_generic_event_t *e) {
  const xcb_key_release_event_t *event = (const xcb_key_release_event_t *)e;

  if(bar_launcher_window(event->event)) {
    shortcut_handle(event, SH_TYPE_LAUNCHER_RELEASE);
#define PRINT         \
  OUT(event->detail); \
  OUT(event->state);  \
  OUT(event->event);
    LOG(TRACE, "event: key_release(launcher)");
#undef PRINT
    return;
  }
  if(mode == MODE_NORMAL) {
    shortcut_handle(event, SH_TYPE_NORMAL_RELEASE);
  } else {
    shortcut_handle(event, SH_TYPE_INSERT_RELEASE);
  }
#define PRINT         \
  OUT(event->detail); \
  OUT(event->state);  \
  OUT_MODE(mode);
  LOG(TRACE, "event: key_release");
#undef PRINT
}

void c_event_button_press(const xcb_generic_event_t *e) {
  const xcb_button_press_event_t *event = (const xcb_button_press_event_t *)e;
  xcb_allow_events(conn, XCB_ALLOW_REPLAY_POINTER, XCB_CURRENT_TIME);
  if((event->detail < 4 || event->detail > 7) && event->event != screen->root) {
    c_focus_xwindow(event->event);
    c_mode_set(MODE_INSERT);
  }
#define PRINT         \
  OUT(event->detail); \
  OUT(event->state);  \
  OUT(event->event);
  LOG(TRACE, "event: button_press");
#undef PRINT
}

void c_event_button_release(const xcb_generic_event_t *e) {
  const xcb_button_release_event_t *event =
    (const xcb_button_release_event_t *)e;
  xcb_allow_events(conn, XCB_ALLOW_REPLAY_POINTER, XCB_CURRENT_TIME);
#define PRINT         \
  OUT(event->detail); \
  OUT(event->state);  \
  OUT(event->event);
  LOG(TRACE, "event: button_release");
#undef PRINT
}

void c_event_create(const xcb_generic_event_t *e) {
  const xcb_create_notify_event_t *event = (const xcb_create_notify_event_t *)e;
  xcb_grab_button(conn, 1, event->window, XCB_EVENT_MASK_BUTTON_PRESS,
                  XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE,
                  XCB_BUTTON_INDEX_ANY, XCB_MOD_MASK_ANY);
  layout_event_create(event->window);
#define PRINT OUT(event->window);
  LOG(TRACE, "event: create_notify");
#undef PRINT
}

void c_event_destroy(const xcb_generic_event_t *e) {
  const xcb_destroy_notify_event_t *event =
    (const xcb_destroy_notify_event_t *)e;
  WINDOW_STATE state_before_destroy = layout_event_destroy(event->window);
  if(state_before_destroy == -1) {
    c_bar_update_minimized();
  } else if(state_before_destroy >= 0) {
    c_bar_update_workspace(state_before_destroy);
  } else {
    c_bar_update_minimized();
  }
#define PRINT         \
  OUT(event->window); \
  OUT_WINDOW_STATE(state_before_destroy);
  LOG(TRACE, "event: destroy_notify");
#undef PRINT
}

void c_event_unmap(const xcb_generic_event_t *e) {
  const xcb_unmap_notify_event_t *event = (const xcb_unmap_notify_event_t *)e;
  bool bar = false;
  size_t workarea_count;
  WINDOW_STATE state;
  size_t bar_container_count;
  const bar_containers_t *bar_containers;
  bar_container_count = bar_get_containers(&bar_containers);
  for(size_t i = 0; i < bar_container_count; i++) {
    if(bar_containers->id[i] == event->window) {
      bar = true;
      break;
    }
  }
  if(!bar) {
    state = layout_event_unmap(event->window);
    workarea_count = layout_workareas(NULL);
    if(state >= 0 && (size_t)state == layout_workspace_focused()) {
      for(size_t i = 0; i < workarea_count; i++) {
        bar_visibility(i, !layout_workspace_area_isfullscreen(state, i));
      }
      c_bar_update_minimized();
    }
  }
  c_focus_restore();

#define PRINT         \
  OUT(event->window); \
  OUT(bar);
  LOG(TRACE, "event: unmap_notify");
#undef PRINT
}

void c_event_focus(const xcb_generic_event_t *e) {
  const xcb_focus_in_event_t *event = (const xcb_focus_in_event_t *)e;
  if(event->detail != XCB_NOTIFY_DETAIL_POINTER) {
    xcb_grab_button(conn, 1, event->event, XCB_EVENT_MASK_BUTTON_PRESS,
                    XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE,
                    XCB_BUTTON_INDEX_ANY, XCB_MOD_MASK_ANY);
    layout_event_focus(event->event);
    hint_window_focused_set(event->event);
#define PRINT OUT(event->event);
    LOG(TRACE, "event: focus_in");
#undef PRINT
  }
  bar_focus(event->event);
}

void c_event_expose(const xcb_generic_event_t *e) {
  const xcb_expose_event_t *event = (const xcb_expose_event_t *)e;
  bar_redraw(event->window);
}

void c_event_xkb(const xcb_generic_event_t *e) {
  shortcut_event_state((xcb_xkb_state_notify_event_t *)e);
}

void c_event_property(const xcb_generic_event_t *e) {
  uint32_t color;
  window_t *win = NULL;
  const xcb_property_notify_event_t *event =
    (const xcb_property_notify_event_t *)e;
  if(hint_atom_urgent(event->atom)) {
    win = layout_xwin2win(event->window);
    bool state = hint_window_urgent(event->window, event->atom);
    if(state) {
      color = border_urgent[c_wm_color_current];
    } else {
      color = border_normal[c_wm_color_current];
    }
    xcb_change_window_attributes(conn, event->window, XCB_CW_BORDER_PIXEL,
                                 &color);
    if(layout_urgency_set(win, state)) {
      if(win->state == WINDOW_ICONIC) {
        c_bar_update_minimized();
      } else if(win->state != (int)layout_workspace_focused()) {
        c_bar_update_workspace(win->state);
      }
#define PRINT       \
  OUT(event->atom); \
  OUT_WINDOW(win);
      LOG(TRACE, "event: property_notify(urgency)");
#undef PRINT
    }
  }
  if(hint_atom_input(event->atom)) {
    if(win == NULL) win = layout_xwin2win(event->window);
    if(layout_input_set(win, hint_window_input(event->window))) {
#define PRINT       \
  OUT(event->atom); \
  OUT_WINDOW(win);
      LOG(TRACE, "event: property_notify(input)");
#undef PRINT
    }
  }
}

void c_event_configure(const xcb_generic_event_t *e) {
  const xcb_configure_request_event_t *event =
    (xcb_configure_request_event_t *)e;
  uint32_t mask = XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT |
                  XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
  uint32_t value_list[4];
  size_t index = 0;
  window_t *win = layout_xwin2win(event->window);
  if(win == NULL) {
    if(event->value_mask & XCB_CONFIG_WINDOW_X) {
      value_list[index++] = event->x;
    }
    if(event->value_mask & XCB_CONFIG_WINDOW_Y) {
      value_list[index++] = event->y;
    }
    if(event->value_mask & XCB_CONFIG_WINDOW_WIDTH) {
      value_list[index++] = event->width;
    }
    if(event->value_mask & XCB_CONFIG_WINDOW_HEIGHT) {
      value_list[index++] = event->height;
    }
    xcb_configure_window(conn, event->window, event->value_mask & mask,
                         value_list);
#define PRINT                   \
  OUT(event->window);           \
  OUT(event->value_mask &mask); \
  OUT_ARR(value_list, index);
    LOG(TRACE, "event: configure_request");
#undef PRINT
  } else {
#define PRINT OUT_WINDOW(win);
    LOG(TRACE, "event: configure_request");
#undef PRINT
  }
}

static void convert_shortcuts(SHORTCUT_TYPE type,
                              config_shortcut_t *conf_shortcuts, size_t len) {
  xcb_mod_mask_t mod_mask;
  for(size_t i = 0; i < len; i++) {
    mod_mask = 0;
    if(conf_shortcuts[i].modifier & MOD_SHIFT) mod_mask |= XCB_MOD_MASK_SHIFT;
    if(conf_shortcuts[i].modifier & MOD_ALT) mod_mask |= XCB_MOD_MASK_1;
    if(conf_shortcuts[i].modifier & MOD_SUPER) mod_mask |= XCB_MOD_MASK_4;
    if(conf_shortcuts[i].modifier & MOD_CTRL) mod_mask |= XCB_MOD_MASK_CONTROL;
    shortcut_add(conf_shortcuts[i].keysym, type, mod_mask,
                 conf_shortcuts[i].function, conf_shortcuts[i].repeatable);
  }
}

static void c_init_bar(rect_t *t_rect, const rect_t *monitors,
                       size_t monitor_count) {
  for(size_t i = 0; i < monitor_count; i++) {
    t_rect[i].x = monitors[i].x;
    t_rect[i].y = monitors[i].y;
    t_rect[i].w = monitors[i].w;
    t_rect[i].h = CONFIG_BAR_HEIGHT;
  }
  bar_init_t binit = (bar_init_t){
    conn,
    screen,
    visual_type,
    .bar_containers = t_rect,
    .bar_container_count = monitor_count,
    layout_workspace_focused,
    CONFIG_BAR_COMPONENT_PADDING,
    CONFIG_BAR_COMPONENT_SEPARATOR,
    CONFIG_BAR_BACKGROUND,
    CONFIG_BAR_FONT,
    COMMON_INIT(CONFIG_BAR_MODE, INSERT, NORMAL),
    {CONFIG_BAR_WORKSPACE_MIN_WIDTH, CONFIG_BAR_WORKSPACE_FOLD,
     SETTINGS_INIT(CONFIG_BAR_WORKSPACE_UNFOCUSED),
     SETTINGS_INIT(CONFIG_BAR_WORKSPACE_FOCUSED),
     SETTINGS_INIT(CONFIG_BAR_WORKSPACE_URGENT), layout_workspace_isempty,
     layout_workspace_isurgent, layout_workspace_name},
    {CONFIG_BAR_INFO_MIN_WIDTH, SETTINGS_INIT(CONFIG_BAR_INFO_NORMAL),
     SETTINGS_INIT(CONFIG_BAR_INFO_HIGHLIGHTED),
     SETTINGS_INIT(CONFIG_BAR_INFO_URGENT),
     (block_info_data_t[])CONFIG_BAR_INFO_BLOCKS,
     LENGTH((block_info_data_t[])CONFIG_BAR_INFO_BLOCKS), system_sh_out},
    {CONFIG_BAR_MINIMIZED_MIN_WIDTH, SETTINGS_INIT(CONFIG_BAR_MINIMIZED_EVEN),
     SETTINGS_INIT(CONFIG_BAR_MINIMIZED_ODD),
     SETTINGS_INIT(CONFIG_BAR_MINIMIZED_URGENT),
     (const plist_t *const *)layout_minimized(), layout_window_lock(),
     offsetof(window_t, name), offsetof(window_t, urgent)},
    {CONFIG_BAR_LAUNCHER_PROMPT_MIN_WIDTH,
     SETTINGS_INIT(CONFIG_BAR_LAUNCHER_PROMPT)},
    COMMON_INIT(CONFIG_BAR_LAUNCHER_HINT, NORMAL, SELECTED)};
  bar_init(&binit);
}

static void c_init_layout(rect_t *t_rect, const rect_t *monitors,
                          size_t monitor_count) {
  for(size_t i = 0; i < monitor_count; i++) {
    t_rect[i].x = monitors[i].x;
    t_rect[i].y = monitors[i].y + CONFIG_BAR_HEIGHT;
    t_rect[i].w = monitors[i].w;
    t_rect[i].h = monitors[i].h - CONFIG_BAR_HEIGHT;
  }
  layout_init_t linit = (layout_init_t){
    conn,
    screen,
    hint_window_class,
    .workareas = t_rect,
    .window_state_changed = hint_window_update_state,
    CONFIG_WORKSPACE_NUMBERS_ONLY,
    .workareas_fullscreen = monitors,
    .workarea_count = monitor_count,
    .name_replacements =
      (const char *const[][2])CONFIG_BAR_MINIMIZED_NAME_REPLACEMENTS,
    .name_replacements_length =
      LENGTH((char *[][2])CONFIG_BAR_MINIMIZED_NAME_REPLACEMENTS),
    .grid_init = {CONFIG_GAPS, CONFIG_BORDERS,
                  (const size_t[])CONFIG_SPAWN_ORDER,
                  LENGTH((size_t[])CONFIG_SPAWN_ORDER)}};
  layout_init(&linit);
}

static void c_init_shortcut(void) {
  config_shortcut_t insert_shortcuts[] = CONFIG_SHORTCUTS_INSERT_MODE;
  config_shortcut_t normal_shortcuts[] = CONFIG_SHORTCUTS_NORMAL_MODE;
  config_shortcut_t launcher_shortcuts[] = CONFIG_SHORTCUTS_LAUNCHER_MODE;
  config_shortcut_t normal_release_shortcuts[] =
    CONFIG_SHORTCUTS_NORMAL_MODE_RELEASE;
  config_shortcut_t insert_release_shortcuts[] =
    CONFIG_SHORTCUTS_INSERT_MODE_RELEASE;
  config_shortcut_t launcher_release_shortcuts[] =
    CONFIG_SHORTCUTS_LAUNCHER_MODE_RELEASE;

  shortcut_init(conn);
  convert_shortcuts(SH_TYPE_NORMAL, normal_shortcuts, LENGTH(normal_shortcuts));
  convert_shortcuts(SH_TYPE_NORMAL_RELEASE, normal_release_shortcuts,
                    LENGTH(normal_release_shortcuts));
  convert_shortcuts(SH_TYPE_INSERT, insert_shortcuts, LENGTH(insert_shortcuts));
  convert_shortcuts(SH_TYPE_INSERT_RELEASE, insert_release_shortcuts,
                    LENGTH(insert_release_shortcuts));
  convert_shortcuts(SH_TYPE_LAUNCHER, launcher_shortcuts,
                    LENGTH(launcher_shortcuts));
  convert_shortcuts(SH_TYPE_LAUNCHER_RELEASE, launcher_release_shortcuts,
                    LENGTH(launcher_release_shortcuts));
  convert_shortcuts(SH_TYPE_NORMAL, normal_shortcuts, LENGTH(normal_shortcuts));
  convert_shortcuts(SH_TYPE_NORMAL_RELEASE, normal_release_shortcuts,
                    LENGTH(normal_release_shortcuts));
}

static void c_init_hint(void) {
  hint_init(&(hint_init_t){conn, screen, layout_workspace_names,
                           layout_workspaces(NULL)});
}

void c_loop(void) {
  run = true;
  while(run) {
    event_next(conn);
    xcb_flush(conn);
    fflush(stdout);
  }
}

void c_init(void) {
  rect_t *monitors;
  size_t monitor_count;
  size_t bar_count;
  char color_out[10];
  const bar_containers_t *bars;
  rect_t *t_rect;
  border_urgent[0] = c_hex2xcolor((color_t)CONFIG_BORDER_URGENT[0]);
  border_urgent[1] = c_hex2xcolor((color_t)CONFIG_BORDER_URGENT[1]);
  border_normal[0] = c_hex2xcolor((color_t)CONFIG_BORDER_NORMAL[0]);
  border_normal[1] = c_hex2xcolor((color_t)CONFIG_BORDER_NORMAL[1]);

  system_init(c_sigterm_action);
  c_init_hint();
  c_init_shortcut();
  LOGE(DEBUG, "Shortcuts init");
  system_monitors(&monitors, &monitor_count);
  t_rect = malloc(monitor_count * sizeof(rect_t));
  c_init_layout(t_rect, monitors, monitor_count);
  LOGE(DEBUG, "Layout init");
  c_init_bar(t_rect, monitors, monitor_count);
  LOGE(DEBUG, "Bar init");
  free(monitors);
  free(t_rect);
  bar_count = bar_get_containers(&bars);
  for(size_t i = 0; i < bar_count; i++) {
    hint_window_hints_set(bars->id[i]);
  }
  c_workspace_switch(hint_saved_workspace_focused());
  c_windows_adopt();
  layout_event_focus(hint_saved_window_focused());

  system_sh_out("color", color_out, 10);
  c_wm_color(strcmp("white\n", color_out) == 0);
  c_mode_set(MODE_NORMAL);
  event_listener_add(XCB_MAP_REQUEST, c_event_map);
  event_listener_add(XCB_MAP_NOTIFY, c_event_map_notify);
  event_listener_add(XCB_KEY_PRESS, c_event_key_press);
  event_listener_add(XCB_KEY_RELEASE, c_event_key_release);
  event_listener_add(XCB_BUTTON_PRESS, c_event_button_press);
  event_listener_add(XCB_CREATE_NOTIFY, c_event_create);
  event_listener_add(XCB_DESTROY_NOTIFY, c_event_destroy);
  event_listener_add(XCB_UNMAP_NOTIFY, c_event_unmap);
  event_listener_add(XCB_FOCUS_IN, c_event_focus);
  event_listener_add(XCB_EXPOSE, c_event_expose);
  event_listener_add(XCB_CLIENT_MESSAGE, c_event_message);
  event_listener_add(XCB_PROPERTY_NOTIFY, c_event_property);
  event_listener_add(system_xkb(), c_event_xkb);
  event_listener_add(XCB_CONFIGURE_REQUEST, c_event_configure);
  xcb_flush(conn);
  fflush(stdout);
}

void c_deinit(void) {
  hint_deinit();
  bar_deinit();
  layout_deinit();
  shortcut_deinit();
  system_deinit();
}
