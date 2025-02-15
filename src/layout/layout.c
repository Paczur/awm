#include "layout.h"

#include <stdio.h>
#include <string.h>

#include "layout_x.h"

#define POSITION_FULLSCREEN 0
#define POSITION_VERT 1
#define POSITION_HOR 2
#define POSITION_QUAR 3

struct monitor {
  u32 x;
  u32 y;
  u32 width;
  u32 height;
  u8 has_above : 1;
  u8 has_below : 1;
  u8 has_to_right : 1;
  u8 has_to_left : 1;
  u8 above;
  u8 below;
  u8 to_right;
  u8 to_left;
};

static u32 workspaces[WORKSPACE_COUNT][WINDOWS_PER_WORKSPACE];
static u8 projection[MAX_MONITOR_COUNT][WINDOWS_PER_WORKSPACE];
static u32 visible_workspaces[MAX_MONITOR_COUNT];
static struct monitor monitors[MAX_MONITOR_COUNT] = {0};
static u32 monitor_count;
static u32 border_focused;
static u32 border_unfocused;
static u32 focused_monitor = -1;
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

static u32 focused_workspace(void) {
  return visible_workspaces[focused_monitor];
}

static void init_monitors(const struct geometry *geoms, u32 m_count) {
  monitor_count = m_count;
  for(u32 i = 0; i < m_count; i++) {
    monitors[i].x = geoms[i].x;
    monitors[i].y = geoms[i].y;
    monitors[i].width = geoms[i].width;
    monitors[i].height = geoms[i].height;
  }
  for(u32 i = 0; i < m_count; i++) {
    for(u32 j = 0; j < m_count; j++) {
      if(i == j) continue;
      if(monitors[i].y == monitors[j].y + monitors[j].height) {
        monitors[i].above = j;
        monitors[j].below = i;
        monitors[i].has_above = 1;
        monitors[j].has_below = 1;
      } else if(monitors[i].y + monitors[i].height == monitors[j].y) {
        monitors[i].below = j;
        monitors[j].above = i;
        monitors[i].has_below = 1;
        monitors[j].has_above = 1;
      }
      if(monitors[i].x == monitors[j].x + monitors[j].width) {
        monitors[i].to_left = j;
        monitors[j].to_right = i;
        monitors[i].has_to_left = 1;
        monitors[j].has_to_right = 1;
      } else if(monitors[i].x + monitors[i].width == monitors[j].x) {
        monitors[i].to_right = j;
        monitors[j].to_left = i;
        monitors[i].has_to_right = 1;
        monitors[j].has_to_left = 1;
      }
    }
  }
}

static void focus_monitor(u32 monitor) {
  if(monitor > monitor_count) return;
  focused_monitor = monitor;
  send_focused_monitor(monitor);
  unfocus_window();
}

void init_layout(const struct geometry *geoms, u32 m_count) {
  u32 *restrict workspace;
  u32 window;
  border_focused = hex2color(BORDER_FOCUSED);
  border_unfocused = hex2color(BORDER_UNFOCUSED);
  init_monitors(geoms, m_count);

  focused_monitor = query_focused_monitor();
  send_focused_monitor(focused_monitor);
  query_visible_workspaces(visible_workspaces, m_count);
  send_visible_workspaces(visible_workspaces, m_count);
  query_workspaces((u32 *)workspaces);
  window = query_focused_window();

  for(u32 j = 0; j < monitor_count; j++) {
    workspace = workspaces[visible_workspaces[j]];
    for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
      listen_to_events(workspace[i]);
      map_window(workspace[i]);
    }
    reconfigure_monitor(j);
  }
  if(window != 0) {
    for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
      if(window == workspace[i]) {
        last_focus = focused_window;
        focused_window = i;
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
  reconfigure_monitor(focused_monitor);
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
      reconfigure_monitor(focused_monitor);
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
  const u32 *const restrict workspace =
    workspaces[visible_workspaces[focused_monitor]];
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    if(window == workspace[i]) {
      last_focus = focused_window;
      focused_window = projection[focused_monitor][i];
      send_focused_window(window);
      change_window_border_color(window, border_focused);
      return;
    }
  }
  for(u32 j = 0; j < monitor_count; j++) {
    for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
      if(window == workspaces[visible_workspaces[j]][i]) {
        focused_monitor = j;
        send_focused_monitor(j);
        last_focus = focused_window;
        focused_window = projection[j][i];
        send_focused_window(window);
        change_window_border_color(window, border_focused);
        return;
      }
    }
  }
}

void focus_out_notify(u32 window) {
  change_window_border_color(window, border_unfocused);
  last_focus = focused_window;
  focused_window = -1;
}

void focus_window_to_left(void) {
  if(focused_window == -1 || focused_window % 2 == 0 ||
     projection[focused_monitor][focused_window] ==
       projection[focused_monitor][focused_window - 1]) {
    if(!monitors[focused_monitor].has_to_left) return;
    const u32 left = monitors[focused_monitor].to_left;
    const u32 index = (focused_window == -1)    ? 0
                      : focused_window % 2 == 1 ? focused_window
                                                : focused_window + 1;
    if(projection[left][index] != WINDOWS_PER_WORKSPACE + 1) {
      focus_window(
        workspaces[visible_workspaces[left]][projection[left][index]]);
    } else {
      focus_monitor(left);
    }
  } else if(focused_window % 2) {
    focus_window(workspaces[focused_workspace()]
                           [projection[focused_monitor][focused_window - 1]]);
  }
}

void focus_window_to_right(void) {
  if(focused_window == -1 || focused_window % 2 ||
     projection[focused_monitor][focused_window] ==
       projection[focused_monitor][focused_window + 1]) {
    if(!monitors[focused_monitor].has_to_right) return;
    const u32 right = monitors[focused_monitor].to_right;
    const u32 index = (focused_window == -1)    ? 0
                      : focused_window % 2 == 0 ? focused_window
                                                : focused_window - 1;
    if(projection[right][index] != WINDOWS_PER_WORKSPACE + 1) {
      focus_window(
        workspaces[visible_workspaces[right]][projection[right][index]]);
    } else {
      focus_monitor(right);
    }
  } else if(focused_window % 2 == 0) {
    focus_window(workspaces[focused_workspace()]
                           [projection[focused_monitor][focused_window + 1]]);
  }
}

void focus_window_above(void) {
  if(focused_window == -1 || focused_window / 2 == 0 ||
     projection[focused_monitor][focused_window] ==
       projection[focused_monitor][focused_window - 2]) {
    if(!monitors[focused_monitor].has_above) return;
    const u32 above = monitors[focused_monitor].above;
    const u32 index = (focused_window == -1)    ? 0
                      : focused_window / 2 == 0 ? focused_window
                                                : focused_window - 2;
    if(projection[above][index] != WINDOWS_PER_WORKSPACE + 1) {
      focus_window(
        workspaces[visible_workspaces[above]][projection[above][index]]);
    } else {
      focus_monitor(monitors[focused_monitor].above);
    }
  } else if(focused_window / 2) {
    focus_window(workspaces[focused_workspace()]
                           [projection[focused_monitor][focused_window + 2]]);
  }
}

void focus_window_below(void) {
  if(focused_window == -1 || focused_window / 2 ||
     projection[focused_monitor][focused_window] ==
       projection[focused_monitor][focused_window + 2]) {
    if(!monitors[focused_monitor].has_below) return;
    const u32 below = monitors[focused_monitor].below;
    const u32 index = (focused_window == -1)    ? 0
                      : focused_window / 2 == 1 ? focused_window
                                                : focused_window + 2;
    if(projection[below][index] != WINDOWS_PER_WORKSPACE + 1) {
      focus_window(
        workspaces[visible_workspaces[below]][projection[below][index]]);
    } else {
      focus_monitor(monitors[focused_monitor].below);
    }
  } else if(focused_window / 2 == 0) {
    focus_window(workspaces[focused_workspace()]
                           [projection[focused_monitor][focused_window + 2]]);
  }
}

void delete_focused_window(void) {
  if(focused_window == -1) return;
  delete_window(workspaces[focused_workspace()][focused_window]);
}

void swap_focused_window_with_left(void) {
  if(focused_window == -1 || focused_window % 2 == 0 ||
     projection[focused_monitor][focused_window] ==
       projection[focused_monitor][focused_window - 1]) {
    if(!monitors[focused_monitor].has_to_left) return;
    const u32 left = monitors[focused_monitor].to_left;
    const u32 index =
      projection[left][(focused_window == -1)    ? 0
                       : focused_window % 2 == 1 ? focused_window
                                                 : focused_window + 1];
    const u32 curr_index = projection[focused_monitor][focused_window];
    last_focus = focused_window;
    if(index != WINDOWS_PER_WORKSPACE + 1) {
      const u32 t = workspaces[focused_workspace()][focused_window];
      workspaces[focused_workspace()][curr_index] =
        workspaces[visible_workspaces[left]][index];
      workspaces[visible_workspaces[left]][index] = t;
      focused_window = index;
    } else {
      workspaces[visible_workspaces[left]][0] =
        workspaces[focused_workspace()][curr_index];
      workspaces[focused_workspace()][curr_index] = 0;
      focused_window = 0;
    }
    send_workspace(workspaces[focused_workspace()], focused_workspace());
    send_workspace(workspaces[visible_workspaces[left]],
                   visible_workspaces[left]);
    reconfigure_monitor(focused_monitor);
    reconfigure_monitor(left);
    focused_monitor = left;
    send_focused_monitor(focused_monitor);
  } else {
    const u32 t = workspaces[focused_workspace()][focused_window];
    workspaces[focused_workspace()][focused_window] =
      workspaces[focused_workspace()][focused_window - 1];
    workspaces[focused_workspace()][focused_window - 1] = t;
    last_focus = focused_window;
    focused_window--;
    send_workspace(workspaces[focused_workspace()], focused_workspace());
    reconfigure_monitor(focused_monitor);
  }
}

void swap_focused_window_with_right(void) {
  if(focused_window == -1 || focused_window % 2 ||
     projection[focused_monitor][focused_window] ==
       projection[focused_monitor][focused_window + 1]) {
    if(!monitors[focused_monitor].has_to_right) return;
    const u32 right = monitors[focused_monitor].to_right;
    const u32 index =
      projection[right][(focused_window == -1)    ? 0
                        : focused_window % 2 == 0 ? focused_window
                                                  : focused_window - 1];
    const u32 curr_index = projection[focused_monitor][focused_window];
    last_focus = focused_window;
    if(index != WINDOWS_PER_WORKSPACE + 1) {
      const u32 t = workspaces[focused_workspace()][curr_index];
      workspaces[focused_workspace()][curr_index] =
        workspaces[visible_workspaces[right]][index];
      workspaces[visible_workspaces[right]][index] = t;
      focused_window = index;
    } else {
      workspaces[visible_workspaces[right]][0] =
        workspaces[focused_workspace()][curr_index];
      workspaces[focused_workspace()][curr_index] = 0;
      focused_window = 0;
    }
    reconfigure_monitor(focused_monitor);
    reconfigure_monitor(right);
    send_workspace(workspaces[focused_workspace()], focused_workspace());
    send_workspace(workspaces[visible_workspaces[right]],
                   visible_workspaces[right]);
    focused_monitor = right;
    send_focused_monitor(focused_monitor);
  } else {
    const u32 t = workspaces[focused_workspace()][focused_window];
    workspaces[focused_workspace()][focused_window] =
      workspaces[focused_workspace()][focused_window + 1];
    workspaces[focused_workspace()][focused_window + 1] = t;
    last_focus = focused_window;
    focused_window++;
    send_workspace(workspaces[focused_workspace()], focused_workspace());
    reconfigure_monitor(focused_monitor);
  }
}

void swap_focused_window_with_above(void) {
  if(focused_window == -1 || focused_window / 2 == 0 ||
     projection[focused_monitor][focused_window] ==
       projection[focused_monitor][focused_window - 2]) {
    if(!monitors[focused_monitor].has_above) return;
    const u32 above = monitors[focused_monitor].above;
    const u32 index =
      projection[above][(focused_window == -1)    ? 0
                        : focused_window / 2 == 0 ? focused_window
                                                  : focused_window + 2];
    const u32 curr_index = projection[focused_monitor][focused_window];
    last_focus = focused_window;
    if(index != WINDOWS_PER_WORKSPACE + 1) {
      const u32 t = workspaces[focused_workspace()][curr_index];
      workspaces[focused_workspace()][curr_index] =
        workspaces[visible_workspaces[above]][index];
      workspaces[visible_workspaces[above]][index] = t;
      focused_window = index;
    } else {
      workspaces[visible_workspaces[above]][0] =
        workspaces[focused_workspace()][curr_index];
      workspaces[focused_workspace()][curr_index] = 0;
      focused_window = 0;
    }
    reconfigure_monitor(focused_monitor);
    reconfigure_monitor(above);
    send_workspace(workspaces[focused_workspace()], focused_workspace());
    send_workspace(workspaces[visible_workspaces[above]],
                   visible_workspaces[above]);
    focused_monitor = above;
    send_focused_monitor(focused_monitor);
  } else {
    const u32 t = workspaces[focused_workspace()][focused_window];
    workspaces[focused_workspace()][focused_window] =
      workspaces[focused_workspace()][focused_window - 2];
    workspaces[focused_workspace()][focused_window - 2] = t;
    last_focus = focused_window;
    focused_window -= 2;
    send_workspace(workspaces[focused_workspace()], focused_workspace());
    reconfigure_monitor(focused_monitor);
  }
}

void swap_focused_window_with_below(void) {
  if(focused_window == -1 || focused_window / 2 ||
     projection[focused_monitor][focused_window] ==
       projection[focused_monitor][focused_window + 2]) {
    if(!monitors[focused_monitor].has_below) return;
    const u32 below = monitors[focused_monitor].below;
    const u32 index =
      projection[below][(focused_window == -1)    ? 0
                        : focused_window / 2 == 1 ? focused_window
                                                  : focused_window - 2];
    const u32 curr_index = projection[focused_monitor][focused_window];
    last_focus = focused_window;
    if(index != WINDOWS_PER_WORKSPACE + 1) {
      const u32 t = workspaces[focused_workspace()][curr_index];
      workspaces[focused_workspace()][curr_index] =
        workspaces[visible_workspaces[below]][index];
      workspaces[visible_workspaces[below]][index] = t;
      focused_window = index;
    } else {
      workspaces[visible_workspaces[below]][0] =
        workspaces[focused_workspace()][curr_index];
      workspaces[focused_workspace()][curr_index] = 0;
      focused_window = 0;
    }
    reconfigure_monitor(focused_monitor);
    reconfigure_monitor(below);
    send_workspace(workspaces[focused_workspace()], focused_workspace());
    send_workspace(workspaces[visible_workspaces[below]],
                   visible_workspaces[below]);
    focused_monitor = below;
    send_focused_monitor(focused_monitor);
  } else {
    const u32 t = workspaces[focused_workspace()][focused_window];
    workspaces[focused_workspace()][focused_window] =
      workspaces[focused_workspace()][focused_window + 2];
    workspaces[focused_workspace()][focused_window + 2] = t;
    last_focus = focused_window;
    focused_window += 2;
    send_workspace(workspaces[focused_workspace()], focused_workspace());
    reconfigure_monitor(focused_monitor);
  }
}
