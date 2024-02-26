#include "controller.h"
#include "system.h"
#include "bar/bar.h"
#include "layout/layout.h"
#include "event.h"
#include "shortcut.h"
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#define XK_LATIN1 //letters
#define XK_MISCELLANY //modifiers and special
#include <X11/keysymdef.h>
#include <X11/XF86keysym.h>

static MODE mode;
#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))
#define MIN(x,y) ((x)<=(y)?(x):(y))

#define SETTINGS_INIT(x) \
{ x ## _BACKGROUND, x ## _FOREGROUND }

#define COMMON_INIT(x, n, h) \
{ x ## _MIN_WIDTH, SETTINGS_INIT(x ## _ ## n), SETTINGS_INIT(x ## _ ## h) }

static void c_bar_update_minimized(void) {
  bar_update_minimized();
}
static void c_bar_update_workspace(size_t n) {
  bar_update_workspace(n);
}
static void c_bar_update_mode(void) {
  bar_update_mode(mode);
}

void c_shutdown(void) { event_stop(); }
void c_workspace_switch(size_t n) {
  layout_switch_workspace(n);
  c_bar_update_workspace(MIN(n, layout_get_focused_workspace()));
}
void c_window_show(size_t n) {
  layout_show(n);
  c_bar_update_minimized();
}
void c_window_focus(size_t n) { layout_focus(n); }
void c_window_focused_swap(size_t n) { layout_swap_focused(n); }
void c_window_focus_down(void) { c_window_focus(layout_below()); }
void c_window_focus_up(void) { c_window_focus(layout_above()); }
void c_window_focus_left(void) { c_window_focus(layout_to_left()); }
void c_window_focus_right(void) { c_window_focus(layout_to_right()); }
void c_window_focused_swap_down(void) { c_window_focused_swap(layout_below()); }
void c_window_focused_swap_up(void) { c_window_focused_swap(layout_above()); }
void c_window_focused_swap_left(void) { c_window_focused_swap(layout_to_left()); }
void c_window_focused_swap_right(void) { c_window_focused_swap(layout_to_right()); }
void c_window_focused_resize_w(int n) { layout_resize_w_focused(n); }
void c_window_focused_resize_h(int n) { layout_resize_h_focused(n); }
void c_window_focused_reset_size(void) { layout_reset_sizes_focused(); }
void c_run(const char* cmd) { system_sh(cmd); }
void c_window_focused_destroy(void) { layout_destroy(); }
void c_window_focused_minimize(void) {
  layout_minimize();
  c_bar_update_minimized();
}
void c_launcher_show(void) { bar_launcher_show(); }
void c_launcher_cancel(void) {
  bar_launcher_hide();
  layout_focus_restore();
  c_mode_set(MODE_NORMAL);
}
void c_launcher_run(void) {
  system_sh(bar_launcher_return());
  layout_focus_restore();
  c_mode_set(MODE_NORMAL);
}
void c_launcher_select_left(void) { bar_launcher_select_left(); }
void c_launcher_select_right(void) { bar_launcher_select_right(); }
void c_launcher_erase(void) { bar_launcher_erase(); }
void c_mode_set(MODE m) {
  if(m == MODE_NORMAL) {
    mode = MODE_NORMAL;
    xcb_ungrab_key(conn, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
    xcb_grab_key(conn, 1, screen->root, XCB_MOD_MASK_ANY, XCB_GRAB_ANY,
                 XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
  } else {
    mode = MODE_INSERT;
    shortcut_enable(conn, screen, SH_TYPE_INSERT_MODE);
  }
  c_bar_update_mode();
}
void c_bar_block_update(size_t n) { bar_update_info(n); }
void c_bar_block_update_highlight(size_t n, int delay) {
  bar_update_info_highlight(n, delay);
}
void c_event_map(const xcb_generic_event_t *e) {
  const xcb_map_request_event_t *event = (const xcb_map_request_event_t*)e;
  if(!layout_event_map(event->window))
    c_bar_update_minimized();
}
void c_event_key_press(const xcb_generic_event_t *e) {
  XKeyEvent keyev;
  char buff[10];
  size_t len;
  const xcb_key_press_event_t *event = (const xcb_key_press_event_t*)e;
  if(bar_launcher_window(event->event)) {
    if(!shortcut_handle(event->detail, SH_TYPE_LAUNCHER, event->state)) {
      keyev = (XKeyEvent) {
        .type = KeyPress,
          .display = dpy,
          .keycode = event->detail,
          .state = event->state
      };
      len = Xutf8LookupString(xic, &keyev, buff, sizeof(buff), NULL, NULL);
      if(len > 0)
        bar_launcher_append(buff, len);
    }
    return;
  }
  if(mode == MODE_INSERT) {
    shortcut_handle(event->detail, SH_TYPE_INSERT_MODE, event->state);
  } else {
    shortcut_handle(event->detail, SH_TYPE_NORMAL_MODE, event->state);
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
  }
}
void c_event_expose(const xcb_generic_event_t *e) {
  const xcb_expose_event_t *event = (const xcb_expose_event_t*)e;
  puts("EXPOSE");
  fflush(stdout);
  bar_redraw(event->window);
}

static void convert_shortcuts(const xcb_get_keyboard_mapping_reply_t *kmapping,
                       SHORTCUT_TYPE type, config_shortcut_t *conf_shortcuts,
                       size_t len) {
  xcb_mod_mask_t mod_mask;
  for(size_t i=0; i<len; i++) {
    mod_mask=-1;
    switch(conf_shortcuts[i].modifier) {
    case MOD_NONE:
      mod_mask = 0;
    break;
    case MOD_SHIFT:
      mod_mask = XCB_MOD_MASK_SHIFT;
    break;
    case MOD_ALT:
      mod_mask = XCB_MOD_MASK_1;
    break;
    case MOD_SUPER:
      mod_mask = XCB_MOD_MASK_4;
    break;
    case MOD_CTRL:
      mod_mask = XCB_MOD_MASK_CONTROL;
    break;
    }
    if(mod_mask != (xcb_mod_mask_t)-1) {
      shortcut_new(kmapping, setup->min_keycode, setup->max_keycode,
                   type, conf_shortcuts[i].keysym, mod_mask,
                   conf_shortcuts[i].function);
    }
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
      layout_workspace_empty, (const plist_t*(*)(void))layout_get_minimized,
      offsetof(window_t, name), CONFIG_BAR_COMPONENT_PADDING,
      CONFIG_BAR_COMPONENT_SEPARATOR, CONFIG_BAR_BACKGROUND, CONFIG_BAR_FONT,
      COMMON_INIT(CONFIG_BAR_MODE, INSERT, NORMAL),
      COMMON_INIT(CONFIG_BAR_WORKSPACE, UNFOCUSED, FOCUSED),
      { CONFIG_BAR_INFO_MIN_WIDTH,
        SETTINGS_INIT(CONFIG_BAR_INFO_NORMAL),
        SETTINGS_INIT(CONFIG_BAR_INFO_HIGHLIGHTED),
        (block_info_data_t[])CONFIG_BAR_INFO_BLOCKS,
        LENGTH((block_info_data_t[])CONFIG_BAR_INFO_BLOCKS), system_sh_out },
      COMMON_INIT(CONFIG_BAR_MINIMIZED, EVEN, ODD),
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
    conn, screen, .workareas = t_rect, .workarea_count = monitor_count,
      .name_replacements = (const char *const [][2])CONFIG_BAR_MINIMIZED_NAME_REPLACEMENTS,
      .name_replacements_length = LENGTH((char*[][2])CONFIG_BAR_MINIMIZED_NAME_REPLACEMENTS),
      .gaps = CONFIG_GAPS, .spawn_order = (const size_t[])CONFIG_SPAWN_ORDER,
      .spawn_order_length = LENGTH((size_t[])CONFIG_SPAWN_ORDER)
  };
  layout_init(&linit);
}

static void c_init_shortcut(xcb_get_keyboard_mapping_cookie_t kmap_cookie) {
  config_shortcut_t insert_shortcuts[] = CONFIG_SHORTCUTS_INSERT_MODE;
  config_shortcut_t normal_shortcuts[] = CONFIG_SHORTCUTS_NORMAL_MODE;
  config_shortcut_t launcher_shortcuts[] = CONFIG_SHORTCUTS_LAUNCHER;
  xcb_get_keyboard_mapping_reply_t *kmap_reply;
  shortcut_init(setup->min_keycode, setup->max_keycode);
  kmap_reply = xcb_get_keyboard_mapping_reply(conn, kmap_cookie, NULL);
  convert_shortcuts(kmap_reply, SH_TYPE_INSERT_MODE,
                    insert_shortcuts, LENGTH(insert_shortcuts));
  convert_shortcuts(kmap_reply, SH_TYPE_NORMAL_MODE,
                    normal_shortcuts, LENGTH(normal_shortcuts));
  convert_shortcuts(kmap_reply, SH_TYPE_LAUNCHER,
                    launcher_shortcuts, LENGTH(launcher_shortcuts));
  free(kmap_reply);
}

void c_loop(void) { event_run(conn); }

void c_init(void) {
  xcb_get_keyboard_mapping_cookie_t kmap_cookie;

  rect_t *monitors;
  size_t monitor_count;
  rect_t *t_rect;

  system_init();
  kmap_cookie = xcb_get_keyboard_mapping(conn, setup->min_keycode,
                                         setup->max_keycode-setup->min_keycode);
  c_init_shortcut(kmap_cookie);

  system_monitors(&monitors, &monitor_count);
  t_rect = malloc(monitor_count*sizeof(rect_t));
  c_init_layout(t_rect, monitors, monitor_count);
  c_init_bar(t_rect, monitors, monitor_count);
  free(monitors);
  free(t_rect);

  c_mode_set(MODE_NORMAL);
  event_listener_add(XCB_MAP_REQUEST, c_event_map);
  event_listener_add(XCB_KEY_PRESS, c_event_key_press);
  event_listener_add(XCB_CREATE_NOTIFY, c_event_create);
  event_listener_add(XCB_DESTROY_NOTIFY, c_event_destroy);
  event_listener_add(XCB_UNMAP_NOTIFY, c_event_unmap);
  event_listener_add(XCB_FOCUS_IN, c_event_focus);
  event_listener_add(XCB_EXPOSE, c_event_expose);
  xcb_flush(conn);
}

void c_deinit(void) {
  bar_deinit();
  layout_deinit();
  shortcut_deinit();
  system_deinit();
}
