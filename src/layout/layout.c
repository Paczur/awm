#include "layout.h"

#include <stdio.h>
#include <string.h>

#include "layout_x.h"

#define POSITION_FULLSCREEN 0
#define POSITION_VERT 1
#define POSITION_HOR 2
#define POSITION_QUAR 3

static u32 workspaces[WORKSPACE_COUNT][WINDOWS_PER_WORKSPACE];
static u8 projection[MAX_MONITOR_COUNT][WINDOWS_PER_WORKSPACE];
static u32 visible_workspaces[MAX_MONITOR_COUNT];
static struct geometry monitors[MAX_MONITOR_COUNT];
static u32 monitor_count;
static u32 border_focused;
static u32 border_unfocused;
static i32 focused_window = 0;
static i32 last_focus = -1;

static u32 hex2color(const char *hex) {
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

static void reconfigure_monitor(u32 monitor) {
  const u32 *const workspace = workspaces[visible_workspaces[monitor]];
  u8 taken[WINDOWS_PER_WORKSPACE * 2];
  u8 index;
  struct geometry geom;
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    projection[monitor][i] = workspace[i] ? i : WINDOWS_PER_WORKSPACE + 1;
    taken[i * 2] = workspace[i] ? 1 : 0;
    taken[i * 2 + 1] = workspace[i] ? 1 : 0;
  }
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    if(projection[monitor][i] != WINDOWS_PER_WORKSPACE + 1) continue;
    index = !(i / 2) * 2 + i % 2;
    if(projection[monitor][index] != index) continue;
    taken[projection[monitor][index] * 2 + 1]++;
    projection[monitor][i] = projection[monitor][index];
  }
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    if(projection[monitor][i] != WINDOWS_PER_WORKSPACE + 1) continue;
    index = i / 2 * 2 + !(i % 2);
    if(projection[monitor][index] != index) continue;
    taken[projection[monitor][index] * 2]++;
    projection[monitor][i] = projection[monitor][index];
  }
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    if(taken[i * 2] == 0) continue;
    geom.width =
      ((taken[i * 2] == 2) ? monitors[monitor].width
                           : monitors[monitor].width / 2 - GAP_SIZE) -
      BORDER_SIZE * 2;
    geom.x = (i % 2 == 0 || taken[i * 2] == 2)
               ? monitors[monitor].x
               : monitors[monitor].x + monitors[monitor].width / 2;
    geom.height =
      ((taken[i * 2 + 1] == 2) ? monitors[monitor].height
                               : monitors[monitor].height / 2 - GAP_SIZE) -
      BORDER_SIZE * 2;
    geom.y = (i / 2 == 0 || taken[i * 2 + 1] == 2)
               ? monitors[monitor].y
               : monitors[monitor].y + monitors[monitor].height / 2;
    configure_window(workspace[i], geom.x, geom.y, geom.width, geom.height,
                     BORDER_SIZE);
  }
}

static i32 is_workspace_empty(u32 w) {
  return !workspaces[w][0] && !workspaces[w][1] && !workspaces[w][2] &&
         !workspaces[w][3];
}

static u32 focused_monitor(void) {
  return focused_window / WINDOWS_PER_WORKSPACE;
}

static u32 focused_workspace(void) {
  return visible_workspaces[focused_monitor()];
}

void init_layout(const struct geometry *geoms, u32 m_count) {
  u32 *restrict workspace;
  u32 window;
  border_focused = hex2color(BORDER_FOCUSED);
  border_unfocused = hex2color(BORDER_UNFOCUSED);
  memcpy(monitors, geoms, sizeof(struct geometry) * m_count);
  monitor_count = m_count;
  query_visible_workspaces(visible_workspaces, m_count);
  send_visible_workspaces(visible_workspaces, m_count);
  query_workspaces((u32 *)workspaces);
  workspace = workspaces[focused_workspace()];
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    listen_to_events(workspace[i]);
    map_window(workspace[i]);
  }
  for(u32 j = 0; j < monitor_count; j++) reconfigure_monitor(j);
  window = query_focused_window();
  if(window == 0) return;
  for(u32 j = 0; j < monitor_count; j++) {
    workspace = workspaces[visible_workspaces[j]];
    for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
      if(window == workspace[i]) {
        last_focus = focused_window;
        focused_window = i + WINDOWS_PER_WORKSPACE * j;
      }
    }
  }
}

void map_request(u32 window) {
  const u32 current_workspace = focused_workspace();
  u32 *const workspace = workspaces[current_workspace];
  const u32 index = !workspace[0]   ? 0
                    : !workspace[1] ? 1
                    : !workspace[3] ? 3
                    : !workspace[2] ? 2
                                    : 4;
  if(index > 3) return;
  workspace[index] = window;
  listen_to_events(window);
  reconfigure_monitor(focused_monitor());
  send_workspace(workspace, current_workspace);
  map_window(window);
  focus_window(window);
}

void unmap_notify(u32 window) {
  const u32 current_workspace = focused_workspace();
  u32 *const workspace = workspaces[current_workspace];
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    if(workspace[i] == window) {
      workspace[i] = 0;
      reconfigure_monitor(focused_monitor());
      send_workspace(workspace, current_workspace);
      break;
    }
  }
  if(last_focus != -1 && workspace[last_focus]) {
    focus_window(workspace[last_focus]);
  } else {
    for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
      if(workspace[i]) {
        focus_window(workspace[i]);
        break;
      }
    }
  }
}

void reset_layout_state(void) {
  for(u32 i = 0; i < WORKSPACE_COUNT; i++) {
    for(u32 j = 0; j < WINDOWS_PER_WORKSPACE; j++) {
      workspaces[i][j] = 0;
    }
  }
  for(u32 i = 0; i < MAX_MONITOR_COUNT; i++) visible_workspaces[i] = i;
  focused_window = -1;
  last_focus = -1;
}

void focus_in_notify(u32 window) {
  const u32 *restrict workspace;
  for(u32 j = 0; j < monitor_count; j++) {
    workspace = workspaces[visible_workspaces[j]];
    for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
      if(window == workspace[i]) {
        last_focus = focused_window;
        focused_window = i + WINDOWS_PER_WORKSPACE * j;
        send_focused_window(window);
        change_window_border_color(window, border_focused);
        break;
      }
    }
  }
}

void focus_out_notify(u32 window) {
  change_window_border_color(window, border_unfocused);
  last_focus = focused_window;
  focused_window = -1;
}

void focus_window_to_left(void) { focus_window_to_right(); }

void focus_window_to_right(void) {
  if(focused_window == -1) return;
  const u32 monitor = focused_monitor();
  const u32 window = focused_window % 4;
  const u32 index = window / 2 * 2 + !(window % 2);
  focus_window(workspaces[focused_workspace()][projection[monitor][index]]);
}

void focus_window_above(void) {
  if(focused_window == -1) return;
  const u32 monitor = focused_monitor();
  const u32 window = focused_window % 4;
  const u32 index = !(window / 2) * 2 + window % 2;
  focus_window(workspaces[focused_workspace()][projection[monitor][index]]);
}

void focus_window_below(void) { focus_window_above(); }

void delete_focused_window(void) {
  if(focused_window == -1) return;
  delete_window(
    workspaces[focused_workspace()][focused_window % WINDOWS_PER_WORKSPACE]);
}

void swap_focused_window_with_right(void) { swap_focused_window_with_left(); }

void swap_focused_window_with_left(void) {
  if(focused_window == -1) return;
  const u32 current_workspace = focused_workspace();
  u32 *restrict const workspace = workspaces[current_workspace];
  const u32 monitor = focused_monitor();
  const u32 index = focused_window / 2 * 2 + !(focused_window % 2);
  const u32 t = workspace[focused_window];
  workspace[focused_window] = workspace[projection[monitor][index]];
  workspace[projection[monitor][index]] = t;
  focused_window = projection[monitor][index];
  reconfigure_monitor(monitor);
}

void swap_focused_window_with_above(void) { swap_focused_window_with_below(); }

void swap_focused_window_with_below(void) {
  if(focused_window == -1) return;
  const u32 current_workspace = focused_workspace();
  u32 *restrict const workspace = workspaces[current_workspace];
  const u32 monitor = focused_monitor();
  const u32 index = !(focused_window / 2) * 2 + focused_window % 2;
  const u32 t = workspace[focused_window];
  workspace[focused_window] = workspace[projection[monitor][index]];
  workspace[projection[monitor][index]] = t;
  focused_window = projection[monitor][index];
  reconfigure_monitor(monitor);
}
