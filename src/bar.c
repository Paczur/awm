#include "bar.h"
#include "global.h"
#include "shared/proc.h"
#include "radix.h"

#include "layout/layout.h"
#include "config.h"
#include "user_config.h"
#include "shortcut.h"
#include <cairo/cairo-xcb.h>
#include <pango/pangocairo.h>
#include <fontconfig/fontconfig.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

radix_node_t *tree;

typedef struct bar_info_t {
  int id;
  char *cmd;
  int timer;
} bar_info_t;

typedef struct bar_info_state_t {
  bool update;
  int status;
  int countdown;
  char text[MAX_INFO_TEXT_LENGTH];
} bar_info_state_t;

size_t bar_count;
char *launcher_prompt;
size_t prompt_i;
uchar *launcher_prompt_size;
size_t launcher_prompt_size_i;
uchar *proper_search;
size_t proper_i;
uchar *proper_search_size;
size_t proper_search_size_i;
size_t last_focus;

search_node_t *last_search;
size_t hint_no = MAX_LAUNCHER_HINTS;
size_t selected;

pthread_t info_thread;
//TODO: PANGO MARKUP SUPPORT
bar_info_state_t info_states[MAX_INFO_BLOCKS] = {0};
bar_info_t info_blocks[MAX_INFO_BLOCKS] = CONFIG_BAR_INFO_BLOCKS;

uint32_t workspace_x;
uint32_t prompt_x;

bool prev_workspaces[MAX_WORKSPACES] = { true, false };
bool prev_minimized[MAX_VISIBLE_MINIMIZED] = { false };
bool prev_hints[MAX_LAUNCHER_HINTS] = { false };
bool prev_info[MAX_INFO_BLOCKS] = { false };
xcb_atom_t wm_class = 0;

typedef struct comp_geom {
  uint32_t x;
  uint32_t width;
  uint32_t text_x;
  uint32_t text_y;
} comp_geom;

void intern_atoms(void) {
  char wm_class_str[] = "WM_CLASS";
  xcb_intern_atom_reply_t *reply = NULL;
  xcb_intern_atom_cookie_t cookie;
  cookie = xcb_intern_atom(conn, 0, LENGTH(wm_class_str)-1, wm_class_str);
  while(reply == NULL) {
    reply = xcb_intern_atom_reply(conn, cookie, NULL);
    if(reply == NULL) {
      cookie = xcb_intern_atom(conn, 0, LENGTH(wm_class_str)-1, wm_class_str);
    }
  }
  wm_class = reply->atom;
  free(reply);
}

void create_component(xcb_window_t *id, xcb_window_t parent) {
  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t values[2] = { view.bar_settings.background, XCB_EVENT_MASK_EXPOSURE };
  *id = xcb_generate_id(conn);
  xcb_create_window(conn, screen->root_depth, *id,
                    parent, 0, 0, 1, view.bar_settings.height, 0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    screen->root_visual, mask, values);
}

void create_text_context(const PangoFontDescription *desc,
                         bar_component_t *comp) {
  comp->surface =
    cairo_xcb_surface_create(conn,
                             comp->id,
                             view.visual_type, 1, view.bar_settings.height);
  comp->cairo = cairo_create(comp->surface);
  comp->pango = pango_cairo_create_layout(comp->cairo);
  pango_layout_set_font_description(comp->pango, desc);
}

void place_bars(void) {
  PangoFontDescription *desc;
  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t values[2] = { view.bar_settings.background, XCB_EVENT_MASK_EXPOSURE };

  for(size_t i=0; i<bar_count; i++) {
    view.bars[i].id = xcb_generate_id(conn);
    xcb_create_window(conn, screen->root_depth, view.bars[i].id,
                      screen->root, view.bars[i].geometry.x,
                      view.bars[i].geometry.y,
                      view.bars[i].geometry.w, view.bars[i].geometry.h,
                      0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->root_visual, mask, values);

    create_component(&view.bars[i].mode.id, view.bars[i].id);

    for(size_t j=0; j<MAX_WORKSPACES; j++) {
      create_component(&view.bars[i].workspaces[j].id, view.bars[i].id);
    }
    for(size_t j=0; j<MAX_VISIBLE_MINIMIZED; j++) {
      create_component(&view.bars[i].minimized[j].id, view.bars[i].id);
    }
    for(size_t j=0; j<MAX_INFO_BLOCKS; j++) {
      create_component(&view.bars[i].info[j].id, view.bars[i].id);
    }

    view.bars[i].launcher.id = xcb_generate_id(conn);
    xcb_create_window(conn, screen->root_depth, view.bars[i].launcher.id,
                      view.bars[i].id, 0, 0, view.bars[i].geometry.w,
                      view.bars[i].geometry.h, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->root_visual, mask, values);
    create_component(&view.bars[i].launcher.prompt.id,
                     view.bars[i].launcher.id);

    for(size_t j=0; j<MAX_LAUNCHER_HINTS; j++) {
      create_component(&view.bars[i].launcher.hints[j].id,
                       view.bars[i].launcher.id);
    }
    xcb_map_window(conn, view.bars[i].id);
    xcb_map_window(conn, view.bars[i].mode.id);
    xcb_map_window(conn, view.bars[i].workspaces[0].id);
    xcb_map_window(conn, view.bars[i].launcher.prompt.id);
  }

  desc = pango_font_description_from_string(view.bar_settings.font);

  for(size_t i=0; i<bar_count; i++) {
    create_text_context(desc, &view.bars[i].mode);
    create_text_context(desc, &view.bars[i].launcher.prompt);

    for(size_t j=0; j<MAX_WORKSPACES; j++) {
      create_text_context(desc, &view.bars[i].workspaces[j]);
    }
    for(size_t j=0; j<MAX_VISIBLE_MINIMIZED; j++) {
      create_text_context(desc, &view.bars[i].minimized[j]);
    }
    for(size_t j=0; j<MAX_LAUNCHER_HINTS; j++) {
      create_text_context(desc, &view.bars[i].launcher.hints[j]);
    }
    for(size_t j=0; j<MAX_INFO_BLOCKS; j++) {
      create_text_context(desc, &view.bars[i].info[j]);
    }
  }
  redraw_bars();

  pango_font_description_free(desc);
}

void set_text(const char *text, const bar_component_t *component) {
  pango_layout_set_text(component->pango, text, -1);
}

void component_geom(const bar_component_t *component,
                    uint32_t min_width, comp_geom *geom) {
  PangoRectangle t;
  PangoRectangle t2;
  pango_layout_get_extents(component->pango, &t, &t2);
  pango_extents_to_pixels(&t, NULL);
  pango_extents_to_pixels(&t2, NULL);
  if(t.x<0) t.x = 0;
  if(t2.y<0) t2.y = 0;
  if((uint)t2.height < view.bar_settings.height) {
    geom->text_y = view.bar_settings.height - t2.height;
    geom->text_y /= 2;
    geom->text_y -= t2.y;
  } else {
    geom->text_y = 0;
  }
  if(t2.width > 0) {
    if((uint)t.x > CONFIG_BAR_COMPONENT_PADDING) {
      geom->width = t2.width + t.x*2;
      geom->text_x = t.x;
    } else {
      geom->width = t2.width + CONFIG_BAR_COMPONENT_PADDING*2;
      geom->text_x = CONFIG_BAR_COMPONENT_PADDING - t.x;
    }
    if(geom->width < min_width) {
      geom->text_x = (min_width - t2.width)/2 - t.x;
      geom->width = min_width;
    }
  } else {
    geom->width = 1;
  }
}

void redraw_component(const comp_geom *geom, const bar_component_t *component,
                      const bar_component_settings_t *settings, size_t m) {
  uint32_t vals[2] = { geom->x, geom->width };
  cairo_move_to(component->cairo, geom->text_x, geom->text_y);
  xcb_clear_area(conn, 0, component->id, 0, 0,
                 view.bars[m].geometry.w, view.bars[m].geometry.h);
  cairo_set_source_rgb(component->cairo,
                       settings->foreground[0],
                       settings->foreground[1],
                       settings->foreground[2]);
  xcb_configure_window(conn, component->id,
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_WIDTH, vals);
  xcb_change_window_attributes(conn, component->id,
                               XCB_CW_BACK_PIXEL, &settings->background);
  cairo_surface_flush(component->surface);
  cairo_xcb_surface_set_size(component->surface, geom->width,
                             view.bars[m].geometry.h);
  pango_cairo_show_layout(component->cairo, component->pango);
}

uint32_t redraw_left_align(const char *text, const bar_component_t *component,
                           const bar_component_settings_t *settings,
                           uint32_t x, uint32_t min_width, size_t m) {
  comp_geom geom;
  geom.x = x;
  if(x != 0)
    geom.x += CONFIG_BAR_COMPONENT_SEPARATOR;
  set_text(text, component);
  component_geom(component, min_width, &geom);
  redraw_component(&geom, component, settings, m);
  if(geom.width > 1)
    geom.x += geom.width;
  return geom.x;
}

//TODO: PROBLEM WITH DISPLAYING DND INDICATOR WHEN NOT PREFIXED BY TEXT
//TODO: SEPARATE REDRAWING FROM WIDTH CALCULATION AND CALCULATE ONLY ON ONE MONITOR
uint32_t redraw_right_align(const char *text, const bar_component_t *component,
                            const bar_component_settings_t *settings,
                            uint32_t x, uint32_t min_width, size_t m) {
  comp_geom geom;
  geom.x = x;
  if(geom.x != view.bars[m].geometry.w)
    geom.x -= CONFIG_BAR_COMPONENT_SEPARATOR;
  set_text(text, component);
  component_geom(component, min_width, &geom);
  if(geom.width > 1)
    geom.x -= geom.width;
  redraw_component(&geom, component, settings, m);
  return geom.x;
}

void populate_name(window_t *window) {
  xcb_get_property_reply_t *reply = NULL;
  const char *classes[][2] = CONFIG_BAR_MINIMIZED_NAME_REPLACEMENTS;
  size_t length[2];
  char *class;
  window->name = calloc(CONFIG_BAR_MINIMIZED_NAME_MAX_LENGTH, sizeof(char));
  xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, window->id, wm_class, XCB_ATOM_STRING,
                     0, 50);
  reply = xcb_get_property_reply(conn, cookie, NULL);

  class = xcb_get_property_value(reply);
  length[1] = xcb_get_property_value_length(reply);
  length[0] = strnlen(class, length[1])+1;
  length[1] -= length[0] + 1;
  for(size_t i=0; classes[i][0] != 0; i++) {
    if(strcmp(class, classes[i][0]) == 0) {
      strcpy(window->name, classes[i][1]);
      free(reply);
      return;
    } else if(strcmp(class+length[0], classes[i][0]) == 0) {
      strcpy(window->name, classes[i][1]);
      free(reply);
      return;
    }
  }
  if(length[1] > CONFIG_BAR_MINIMIZED_NAME_MAX_LENGTH) {
    memcpy(window->name, class+length[0], CONFIG_BAR_MINIMIZED_NAME_MAX_LENGTH);
    if(window->name[CONFIG_BAR_MINIMIZED_NAME_MAX_LENGTH-1] != 0) {
      window->name[CONFIG_BAR_MINIMIZED_NAME_MAX_LENGTH-1] = 0;
      window->name[CONFIG_BAR_MINIMIZED_NAME_MAX_LENGTH-2] = '.';
      window->name[CONFIG_BAR_MINIMIZED_NAME_MAX_LENGTH-3] = '.';
      window->name[CONFIG_BAR_MINIMIZED_NAME_MAX_LENGTH-4] = '.';
    }
  } else {
    memcpy(window->name, class+length[0], length[1]);
  }
  if(window->name[0] < 'A' ||
     window->name[0] > 'z' ||
     (window->name[0] > 'Z' &&
      window->name[0] < 'a')) {
    window->name[0] = '?';
    window->name[1] = 0;
  }
  free(reply);
  return;
}

void redraw_minimized(void) {
  comp_geom geom[MAX_VISIBLE_MINIMIZED];
  size_t len = 0;
  size_t width = 0;
  const window_list_t *node = layout_get_minimized();
  while(node != NULL) {
    //synchronous because windows are added one by one
    if(node->window->name == NULL)
      populate_name(node->window);
    set_text(node->window->name, view.bars[0].minimized+len);
    component_geom( view.bars[0].minimized+len,
                    CONFIG_BAR_MINIMIZED_MIN_WIDTH,
                    geom+len);
    width += geom[len].width;
    node = node->next;
    len++;
  }
  width += CONFIG_BAR_COMPONENT_SEPARATOR*(len-1);

  for(size_t j=0; j<len; j++) {
    if(!prev_minimized[j]) {
      for(size_t i=0; i<bar_count; i++)
        xcb_map_window(conn, view.bars[i].minimized[j].id);
      prev_minimized[j] = true;
    }
  }
  for(size_t j=len; j<MAX_VISIBLE_MINIMIZED; j++) {
    if(prev_minimized[j]) {
      for(size_t i=0; i<bar_count; i++)
        xcb_unmap_window(conn, view.bars[i].minimized[j].id);
      prev_minimized[j] = false;
    }
  }

  if(len == 0) return;
  for(size_t i=0; i<bar_count; i++) {
    geom[0].x = (view.bars[i].geometry.w - width)/2;
    redraw_component(geom, view.bars[i].minimized,
                     &view.bar_settings.minimized_odd, i);
    for(size_t j=1; j<len; j++) {
      geom[j].x = geom[j-1].x + geom[j-1].width + CONFIG_BAR_COMPONENT_SEPARATOR;
      if(j%2 != 0) {
        redraw_component(geom+j, view.bars[i].minimized+j,
                         &view.bar_settings.minimized_even, i);
      } else {
        redraw_component(geom+j, view.bars[i].minimized+j,
                         &view.bar_settings.minimized_odd, i);
      }
    }
  }
}

void redraw_workspaces(void) {
  char num[2] = { '1', 0 };
  size_t iterator = 1;
  uint32_t xs[11] = { workspace_x, 0 };
  size_t focused_workspace = layout_get_focused_workspace();
  for(size_t i=0; i<MAX_WORKSPACES; i++) {
    if(i == 9) {
      num[0] = '0';
    } else {
      num[0] = i+'1';
    }
    if(i == focused_workspace) {
      if(prev_workspaces[i] == false) {
        for(size_t j=0; j<bar_count; j++)
          xcb_map_window(conn, view.bars[j].workspaces[i].id);
      }
      for(size_t j=0; j<bar_count; j++) {
        xs[iterator] = redraw_left_align(num, view.bars[j].workspaces+i,
                                         &view.bar_settings.workspace_focused,
                                         xs[iterator-1],
                                         CONFIG_BAR_WORKSPACE_MIN_WIDTH,
                                         j);
      }
      prev_workspaces[i] = true;
      iterator++;
    } else if(!layout_workspace_empty(i)) {
      if(prev_workspaces[i] == false) {
        for(size_t j=0; j<bar_count; j++)
          xcb_map_window(conn, view.bars[j].workspaces[i].id);
      }
      for(size_t j=0; j<bar_count; j++) {
        xs[iterator] = redraw_left_align(num, view.bars[j].workspaces+i,
                                         &view.bar_settings.workspace_unfocused,
                                         xs[iterator-1],
                                         CONFIG_BAR_WORKSPACE_MIN_WIDTH,
                                         j);
      }
      prev_workspaces[i] = true;
      iterator++;
    } else if(prev_workspaces[i]) {
      for(size_t j=0; j<bar_count; j++)
        xcb_unmap_window(conn, view.bars[j].workspaces[i].id);
      prev_workspaces[i] = false;
    }
  }
}

void redraw_mode(void) {
  if(mode == MODE_NORMAL) {
    for(size_t i=0; i<bar_count; i++) {
      workspace_x = redraw_left_align("󰆾", &view.bars[i].mode,
                                      &view.bar_settings.mode_normal, 0,
                                      CONFIG_BAR_WORKSPACE_MIN_WIDTH,
                                      i);
    }
  } else {
    for(size_t i=0; i<bar_count; i++) {
      workspace_x = redraw_left_align("󰗧", &view.bars[i].mode,
                                      &view.bar_settings.mode_insert, 0,
                                      CONFIG_BAR_WORKSPACE_MIN_WIDTH,
                                      i);
    }
  }
  redraw_workspaces();
}

void redraw_info(void) {
  uint32_t x[MAX_INFO_BLOCKS];
  for(int i=MAX_INFO_BLOCKS-1; i>=0; i--) {
    if(!prev_info[i] && info_states[i].text[0] != 0) {
      for(size_t j=0; j<bar_count; j++) {
        xcb_map_window(conn, view.bars[j].info[i].id);
      }
      prev_info[i] = true;
    } else if(prev_info[i] && info_states[i].text[0] == 0) {
      for(size_t j=0; j<bar_count; j++) {
        xcb_unmap_window(conn, view.bars[j].info[i].id);
      }
      prev_info[i] = false;
    }
  }
  for(size_t i=0; i<bar_count; i++) {
    x[MAX_INFO_BLOCKS-1] = view.bars[i].geometry.w;
    for(size_t j=MAX_INFO_BLOCKS-1; j>0; j--) {
      x[j-1] = redraw_right_align(info_states[j].text, view.bars[i].info+j,
                                  (info_states[j].status == 1 ||
                                   info_states[j].status == 33) ?
                                  &view.bar_settings.info_highlighted :
                                  &view.bar_settings.info,
                                  x[j], CONFIG_BAR_INFO_MIN_WIDTH,
                                  i);
    }
    redraw_right_align(info_states[0].text, view.bars[i].info,
                       (info_states[0].status==1) ?
                       &view.bar_settings.info_highlighted :
                       &view.bar_settings.info,
                       x[0], CONFIG_BAR_INFO_MIN_WIDTH,
                       i);
  }
}

void update_info_n_highlight(int n, int delay) {
  for(size_t i=0; i<MAX_INFO_BLOCKS; i++) {
    if(info_blocks[i].id == n) {
      info_states[i].countdown = delay*CLOCK_STEPS_PER_SECOND;
      info_states[i].update = true;
      break;
    }
  }
}

void update_info_n(int n) {
  for(size_t i=0; i<MAX_INFO_BLOCKS; i++) {
    if(info_blocks[i].id == n) {
      info_states[i].countdown = 1;
      break;
    }
  }
}

void *update_info(void*) {
  bool updated;
  struct timespec ts =
    (struct timespec) { .tv_nsec = 1000000000/CLOCK_STEPS_PER_SECOND };
  for(size_t i=0; i<MAX_INFO_BLOCKS; i++) {
    if(info_blocks[i].cmd == NULL) {
      info_states[i].countdown = -1;
    }
  }
  while(true) {
    updated = false;
    for(size_t i=0; i<MAX_INFO_BLOCKS; i++) {
      if(info_states[i].update) {
        shout(info_blocks[i].cmd, info_states[i].text, MAX_INFO_TEXT_LENGTH);
        info_states[i].status = 1;
        info_states[i].update = false;
        updated = true;
      } else if(info_states[i].countdown == 0) {
        info_states[i].status = shout(info_blocks[i].cmd, info_states[i].text,
                                      MAX_INFO_TEXT_LENGTH);
        info_states[i].countdown = info_blocks[i].timer*CLOCK_STEPS_PER_SECOND;
        updated = true;
      } else if(info_states[i].countdown > 0) {
        info_states[i].countdown--;
      }
    }
    if(updated) {
      redraw_info();
      xcb_flush(conn);
    }
    nanosleep(&ts, &ts);
  }
}

void redraw_bars(void) {
  redraw_mode();
  redraw_minimized();
  redraw_info();
}

void bar_init(rect_t *bs, size_t count) {
  view.bars = malloc(sizeof(bar_t) * count);
  bar_count = count;
  for(size_t i=0; i<count; i++) {
    view.bars[i].geometry = bs[i];
  }
  intern_atoms();
  place_bars();
  pthread_create(&info_thread, NULL, update_info, NULL);
  radix_populate(&tree);
}

void destroy_component(bar_component_t *comp) {
  cairo_destroy(comp->cairo);
  comp->cairo = NULL;
  cairo_surface_destroy(comp->surface);
  comp->surface = NULL;
  g_object_unref(comp->pango);
  comp->pango = NULL;
}

void bar_deinit(void) { for(size_t i=0; i<bar_count; i++) {
    destroy_component(&view.bars[i].mode);
    destroy_component(&view.bars[i].launcher.prompt);
    for(size_t j=0; j<MAX_WORKSPACES; j++) {
      destroy_component(&view.bars[i].workspaces[j]);
    }
    for(size_t j=0; j<MAX_VISIBLE_MINIMIZED; j++) {
      destroy_component(&view.bars[i].minimized[j]);
    }
    for(size_t j=0; j<MAX_LAUNCHER_HINTS; j++) {
      destroy_component(&view.bars[i].launcher.hints[j]);
    }
    for(size_t j=0; j<MAX_INFO_BLOCKS; j++) {
      destroy_component(&view.bars[i].info[j]);
    }
  }
  pango_cairo_font_map_set_default(NULL);
  cairo_debug_reset_static_data();
  FcFini();
  free(view.bars);
  view.bars = NULL;
  radix_clear(tree);
  tree = NULL;
}

void redraw_prompt(void) {
  comp_geom geom;
  geom.x = 0;
  set_text(launcher_prompt, &view.bars[0].launcher.prompt);
  component_geom(&view.bars[0].launcher.prompt,
                 CONFIG_BAR_LAUNCHER_PROMPT_MIN_WIDTH, &geom);
  for(size_t i=0; i<bar_count; i++) {
    set_text(launcher_prompt, &view.bars[0].launcher.prompt);
    geom.text_x = CONFIG_BAR_COMPONENT_PADDING;
    redraw_component(&geom, &view.bars[i].launcher.prompt,
                     &view.bar_settings.launcher_prompt, i);
  }
  prompt_x = geom.width + geom.x + CONFIG_BAR_COMPONENT_SEPARATOR;
}

void redraw_hints(void) {
  comp_geom geom[MAX_LAUNCHER_HINTS];
  size_t width = 0;
  hint_no = 0;
  for(size_t i=0; i<MAX_LAUNCHER_HINTS && radix_hints[hint_no][0] != 0; i++) {
    set_text(radix_hints[i], view.bars[0].launcher.hints+i);
    component_geom(view.bars[0].launcher.hints+i,
                   CONFIG_BAR_LAUNCHER_HINT_MIN_WIDTH, geom+i);
    for(size_t j=0; j<bar_count; j++) {
      if(width + geom[i].width + CONFIG_BAR_COMPONENT_SEPARATOR
         > view.bars[j].geometry.w - prompt_x) {
        goto skip_hints;
      }
      set_text(radix_hints[i], view.bars[j].launcher.hints+i);
    }
    width += geom[i].width + CONFIG_BAR_COMPONENT_SEPARATOR;
    hint_no++;
  }
skip_hints:

  for(size_t i=0; i<hint_no; i++) {
    if(!prev_hints[i]) {
      for(size_t j=0; j<bar_count; j++) {
        xcb_map_window(conn, view.bars[j].launcher.hints[i].id);
      }
      prev_hints[i] = true;
    }
  }
  for(size_t i=hint_no; i<MAX_LAUNCHER_HINTS; i++) {
    if(prev_hints[i]) {
      for(size_t j=0; j<bar_count; j++) {
        xcb_unmap_window(conn, view.bars[j].launcher.hints[i].id);
      }
      prev_hints[i] = false;
    }
  }

  if(hint_no > 0) {
    for(size_t i=0; i<bar_count; i++) {
      geom[0].x = (view.bars[i].geometry.w - width)/2;
      geom[0].x = (geom[0].x < prompt_x) ? prompt_x : geom[0].x;
      if(selected == 0) {
        redraw_component(geom, view.bars[i].launcher.hints,
                         &view.bar_settings.launcher_hint_selected, i);
      } else {
        redraw_component(geom, view.bars[i].launcher.hints,
                         &view.bar_settings.launcher_hint, i);
      }
      if(selected < hint_no && selected > 0) {
        for(size_t j=1; j<selected; j++) {
          geom[j].x = geom[j-1].x + geom[j-1].width + CONFIG_BAR_COMPONENT_SEPARATOR;
          redraw_component(geom+j, view.bars[i].launcher.hints+j,
                           &view.bar_settings.launcher_hint, i);
        }
        geom[selected].x = geom[selected-1].x + geom[selected-1].width
          + CONFIG_BAR_COMPONENT_SEPARATOR;
        redraw_component(geom+selected, view.bars[i].launcher.hints+selected,
                         &view.bar_settings.launcher_hint_selected, i);
        for(size_t j=selected+1; j<hint_no; j++) {
          geom[j].x = geom[j-1].x + geom[j-1].width + CONFIG_BAR_COMPONENT_SEPARATOR;
          redraw_component(geom+j, view.bars[i].launcher.hints+j,
                           &view.bar_settings.launcher_hint, i);
        }
      } else {
        for(size_t j=1; j<hint_no; j++) {
          geom[j].x = geom[j-1].x + geom[j-1].width + CONFIG_BAR_COMPONENT_SEPARATOR;
          redraw_component(geom+j, view.bars[i].launcher.hints+j,
                           &view.bar_settings.launcher_hint, i);
        }
      }
      //TODO: No idea why it doesn't work when i remove this loop
      for(size_t j=0; j<hint_no; j++) {
        pango_cairo_show_layout(view.bars[i].launcher.hints[j].cairo,
                                view.bars[i].launcher.hints[j].pango);
      }
    }
  }
}

void hide_launcher(void) {
  for(size_t i=0; i<bar_count; i++) {
    xcb_unmap_window(conn, view.bars[i].launcher.id);
  }
  free(launcher_prompt);
  free(proper_search);
  free(launcher_prompt_size);
  free(proper_search_size);
  redraw_bars();
  layout_focus_restore();
  normal_mode();
}

void redraw_launcher(void) {
  redraw_prompt();
  redraw_hints();
}

void show_launcher(void) {
  char buff[LAUNCHER_PROMPT_LENGTH];

  launcher_prompt = malloc(LAUNCHER_PROMPT_LENGTH);
  proper_search = malloc(LAUNCHER_PROMPT_LENGTH);
  launcher_prompt_size = malloc(LAUNCHER_PROMPT_LENGTH);
  proper_search_size = malloc(LAUNCHER_PROMPT_LENGTH);

  prompt_i = 0;
  proper_i = 0;
  selected = 0;
  last_search = NULL;
  launcher_prompt_size_i = 0;
  proper_search_size_i = 0;

  for(size_t i=0; i<bar_count; i++) {
    xcb_map_window(conn, view.bars[i].launcher.id);
  }
  memset(launcher_prompt, 0, LAUNCHER_PROMPT_LENGTH);
  radix_unmark(tree);
  radix_populate(&tree);
  radix_gen_hints(tree, buff, 0);
  redraw_launcher();

  xcb_ungrab_key(conn, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
  xcb_grab_key(conn, 1, view.bars[0].launcher.prompt.id, XCB_MOD_MASK_ANY,
               XCB_GRAB_ANY, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                      view.bars[0].launcher.prompt.id, XCB_CURRENT_TIME);
}

void confirm_launcher(void) {
  hide_launcher();
  insert_mode();
  sh(radix_hints[selected]);
}

void hint_direction(size_t n) {
  selected = (selected+n+hint_no)%hint_no;
  redraw_hints();
}

void prompt_erase(void) {
  char buff[LAUNCHER_PROMPT_LENGTH];
  if(launcher_prompt_size_i-1 < launcher_prompt_size_i) {
    prompt_i -= launcher_prompt_size[--launcher_prompt_size_i];
    proper_i -= proper_search_size[--proper_search_size_i];
    launcher_prompt[prompt_i] = 0;
    proper_search[proper_i] = 0;
    if(last_search != NULL) {
      free(last_search);
    }
    if(proper_i > 0) {
      selected = 0;
      memcpy(buff, proper_search, proper_i);
      last_search = radix_search(tree, buff, proper_i);
      radix_gen_hints_sr(last_search, buff, proper_i);
    } else {
      last_search = NULL;
      radix_gen_hints(tree, buff, 0);
    }
    redraw_launcher();
  }
}

void launcher_keypress(const xcb_key_press_event_t *event) {
  char buff[LAUNCHER_PROMPT_LENGTH];
  search_node_t *search;
  size_t len;
  bool found;
  xcb_keycode_t keycode;
  XKeyEvent keyev;
  keycode = event->detail;
  found = shortcut_handle(keycode, SH_TYPE_LAUNCHER, event->state);
  if(found) return;
  if(prompt_i >= LAUNCHER_PROMPT_LENGTH-1) return;
  keyev = (XKeyEvent) {
    .type = KeyPress,
      .display = dpy,
      .keycode = keycode,
      .state = event->state
  };
  len = Xutf8LookupString(xic, &keyev, buff, sizeof(buff), NULL, NULL);
  if(len > 0) {
    selected = 0;
    launcher_prompt_size[launcher_prompt_size_i++] = len;
    if(prompt_i + len > LAUNCHER_PROMPT_LENGTH) {
      memcpy(launcher_prompt+prompt_i, buff, LAUNCHER_PROMPT_LENGTH-prompt_i);
    } else {
      memcpy(launcher_prompt+prompt_i, buff, len);
    }
    prompt_i += len;
    if(prompt_i < LAUNCHER_PROMPT_LENGTH) {
      launcher_prompt[prompt_i] = 0;
    }

    if(last_search != NULL) {
      search = radix_search_sr(last_search, buff, len);
    } else {
      search = radix_search(tree, buff, len);
    }

    if(search != NULL) {
      if(last_search != search)
        free(last_search);
      last_search = search;
      memcpy(proper_search+proper_i, buff, len-last_search->wrong);
      proper_i += len-last_search->wrong;
      proper_search_size[proper_search_size_i++] = len-last_search->wrong;
      memcpy(buff, proper_search, proper_i);
      proper_search[proper_i] = 0;
      last_search->wrong = 0;
      radix_gen_hints_sr(last_search, buff, proper_i);
    } else {
      proper_search_size[proper_search_size_i++] = 0;
    }
    redraw_launcher();
  }
}
