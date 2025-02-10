#include "layout.h"

#include <stdio.h>
#include <string.h>

#include "layout_x.h"

#define POSITION_FULLSCREEN 0
#define POSITION_VERT 1
#define POSITION_HOR 2
#define POSITION_QUAR 3

static u32 workspaces[WORKSPACE_COUNT][WINDOWS_PER_WORKSPACE];
static u32 projection[MAX_MONITOR_COUNT][WINDOWS_PER_WORKSPACE];
static u32 current_workspace;
static struct geometry monitors[MAX_MONITOR_COUNT];
static u32 border_focused;
static u32 border_unfocused;
static i32 focused_window = -1;
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

static void position(u32 win, u32 pos, u32 type) {
  const struct geometry *const monitor = monitors;
  const u32 x = monitor->x;
  const u32 y = monitor->y;
  const u32 width = monitor->width;
  const u32 height = monitor->height;
  const u32 position[4][2] = {{x, y},
                              {x + width / 2, y},
                              {x, y + height / 2},
                              {x + width / 2, y + height / 2}};
  const u32 lay[4][2] = {
    {width - BORDER_SIZE * 2, height - BORDER_SIZE * 2},
    {width / 2 - BORDER_SIZE * 2, height - BORDER_SIZE * 2},
    {width - BORDER_SIZE * 2, height / 2 - BORDER_SIZE * 2},
    {width / 2 - BORDER_SIZE * 2, height / 2 - BORDER_SIZE * 2},
  };
  configure_window(win, position[pos][0], position[pos][1], lay[type][0],
                   lay[type][1], BORDER_SIZE);
}

static void reconfigure_workspace(u32 w) {
  const u32 *const workspace = workspaces[w];
  const u32 window_count =
    !!workspace[0] + !!workspace[1] + !!workspace[2] + !!workspace[3];
  const u32 monitor = 0;
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++)
    projection[monitor][i] = workspace[i] ? i : WINDOWS_PER_WORKSPACE + 1;
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    if(projection[monitor][i] != WINDOWS_PER_WORKSPACE + 1) continue;
    if(projection[monitor][!(i / 2) * 2 + i % 2] == WINDOWS_PER_WORKSPACE + 1)
      continue;
    projection[monitor][i] = !(i / 2) * 2 + i % 2;
  }
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    if(projection[monitor][i] != WINDOWS_PER_WORKSPACE + 1) continue;
    if(projection[monitor][i / 2 * 2 + !(i % 2)] == WINDOWS_PER_WORKSPACE + 1)
      continue;
    projection[monitor][i] = i / 2 * 2 + !(i % 2);
  }
  if(window_count == 1) {
    const u32 index = workspace[0]   ? 0
                      : workspace[1] ? 1
                      : workspace[2] ? 2
                      : workspace[3] ? 3
                                     : 0;
    position(workspace[index], 0, POSITION_FULLSCREEN);
  } else if(window_count == 4) {
    for(u32 i = 0; i < 4; i++) position(workspace[i], i, POSITION_QUAR);
  } else if(window_count == 2) {
    if(!!workspace[0] == !!workspace[2]) {  // one above other
      const u32 offset = workspace[0] ? 0 : 1;
      position(workspace[0 + offset], 0, POSITION_HOR);
      position(workspace[2 + offset], 2, POSITION_HOR);
    } else {
      const u32 indexes[2] = {workspace[0] ? 0 : 2, workspace[1] ? 1 : 3};
      position(workspace[indexes[0]], 0, POSITION_VERT);
      position(workspace[indexes[1]], 1, POSITION_VERT);
    }
  } else if(window_count == 3) {
    if(!!workspace[0] != !!workspace[2]) {  // left is slice
      if(workspace[0]) {
        position(workspace[0], 0, POSITION_VERT);
        position(workspace[1], 1, POSITION_QUAR);
        position(workspace[3], 3, POSITION_QUAR);
      } else {
        position(workspace[1], 1, POSITION_QUAR);
        position(workspace[2], 0, POSITION_VERT);
        position(workspace[3], 3, POSITION_QUAR);
      }
    } else {  // right is slice
      if(workspace[1]) {
        position(workspace[0], 0, POSITION_QUAR);
        position(workspace[1], 1, POSITION_VERT);
        position(workspace[2], 2, POSITION_QUAR);
      } else {
        position(workspace[0], 0, POSITION_QUAR);
        position(workspace[2], 2, POSITION_QUAR);
        position(workspace[3], 1, POSITION_VERT);
      }
    }
  }
}

static i32 is_workspace_empty(u32 w) {
  return !workspaces[w][0] && !workspaces[w][1] && !workspaces[w][2] &&
         !workspaces[w][3];
}

void init_layout(const struct geometry *geoms, u32 monitor_count) {
  border_focused = hex2color(BORDER_FOCUSED);
  border_unfocused = hex2color(BORDER_UNFOCUSED);
  memcpy(monitors, geoms, sizeof(struct geometry) * monitor_count);
  current_workspace = query_current_workspace();
  query_workspaces((u32 *)workspaces);
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++)
    map_window(workspaces[current_workspace][i]);
  focus_window(query_focused_window());
}

void map_request(u32 window) {
  u32 *const workspace = workspaces[current_workspace];
  const u32 index = !workspace[0]   ? 0
                    : !workspace[1] ? 1
                    : !workspace[3] ? 3
                    : !workspace[2] ? 2
                                    : 4;
  if(index > 3) return;
  workspace[index] = window;
  listen_to_events(window);
  reconfigure_workspace(current_workspace);
  send_workspace(workspace, current_workspace);
  map_window(window);
  focus_window(window);
}

void unmap_notify(u32 window) {
  u32 *const workspace = workspaces[current_workspace];
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    if(workspace[i] == window) {
      workspace[i] = 0;
      reconfigure_workspace(current_workspace);
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
  current_workspace = 0;
  focused_window = -1;
  last_focus = -1;
}

void focus_in_notify(u32 window) {
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    if(window == workspaces[current_workspace][i]) {
      last_focus = focused_window;
      focused_window = i;
      send_focused_window(window);
      change_window_border_color(window, border_focused);
      break;
    }
  }
}

void focus_out_notify(u32 window) {
  change_window_border_color(window, border_unfocused);
  last_focus = focused_window;
  focused_window = -1;
}

void focus_window_to_left(void) {
  if(focused_window == -1) return;
  const u32 monitor = 0;
  const u32 index = focused_window / 2 * 2 + !(focused_window % 2);
  focus_window(workspaces[current_workspace][projection[monitor][index]]);
}

void focus_window_to_right(void) {
  if(focused_window == -1) return;
  const u32 monitor = 0;
  const u32 index = focused_window / 2 * 2 + !(focused_window % 2);
  focus_window(workspaces[current_workspace][projection[monitor][index]]);
}

void focus_window_above(void) {
  if(focused_window == -1) return;
  const u32 monitor = 0;
  const u32 index = !(focused_window / 2) * 2 + focused_window % 2;
  focus_window(workspaces[current_workspace][projection[monitor][index]]);
}

void focus_window_below(void) {
  if(focused_window == -1) return;
  const u32 monitor = 0;
  const u32 index = !(focused_window / 2) * 2 + focused_window % 2;
  focus_window(workspaces[current_workspace][projection[monitor][index]]);
}
