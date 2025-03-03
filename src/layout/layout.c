#include "layout.h"

#include <stdio.h>
#include <string.h>

#include "../const.h"
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
static i32 focused_windows[WORKSPACE_COUNT] = {-1, -1, -1, -1, -1,
                                               -1, -1, -1, -1, -1};
static u32 monitor_count;
static u32 focused_monitor = -1;
static u32 minimized_windows[MINIMIZE_QUEUE_SIZE];
static u32 minimized_window_count;

static void expand_height(u32 monitor, u8 *taken, u8 index) {
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    if(projection[monitor][i] != WINDOWS_PER_WORKSPACE) continue;
    index = !(i / 2) * 2 + i % 2;
    if(projection[monitor][index] != index) continue;
    taken[projection[monitor][index] * 2 + 1]++;
    projection[monitor][i] = projection[monitor][index];
  }
}

static void expand_width(u32 monitor, u8 *taken, u8 index) {
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    if(projection[monitor][i] != WINDOWS_PER_WORKSPACE) continue;
    index = i / 2 * 2 + !(i % 2);
    if(projection[monitor][index] != index) continue;
    taken[projection[monitor][index] * 2]++;
    projection[monitor][i] = projection[monitor][index];
  }
}

static void reconfigure_monitor(u32 monitor) {
  const u32 *const workspace = workspaces[visible_workspaces[monitor]];
  u8 taken[WINDOWS_PER_WORKSPACE * 2];
  u8 index;
  struct geometry geom;
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    projection[monitor][i] = workspace[i] ? i : WINDOWS_PER_WORKSPACE;
    taken[i * 2] = workspace[i] ? 1 : 0;
    taken[i * 2 + 1] = workspace[i] ? 1 : 0;
  }
  if(monitors[monitor].height > monitors[monitor].width) {
    expand_width(monitor, taken, index);
    expand_height(monitor, taken, index);
  } else {
    expand_height(monitor, taken, index);
    expand_width(monitor, taken, index);
  }
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    if(taken[i * 2] == 0) continue;
    geom.width =
      ((taken[i * 2] == 2) ? monitors[monitor].width - GAP_SIZE * 2
                           : monitors[monitor].width / 2 - GAP_SIZE * 1.5) -
      BORDER_SIZE * 2;
    geom.x =
      (i % 2 == 0 || taken[i * 2] == 2)
        ? monitors[monitor].x + GAP_SIZE
        : monitors[monitor].x + monitors[monitor].width / 2 + GAP_SIZE / 2;
    geom.height = ((taken[i * 2 + 1] == 2)
                     ? monitors[monitor].height - GAP_SIZE * 2
                     : monitors[monitor].height / 2 - GAP_SIZE * 1.5) -
                  BORDER_SIZE * 2;
    geom.y =
      (i / 2 == 0 || taken[i * 2 + 1] == 2)
        ? monitors[monitor].y + GAP_SIZE
        : monitors[monitor].y + monitors[monitor].height / 2 + GAP_SIZE / 2;
    if(taken[i * 2] == 2 && taken[i * 2 + 1] == 2) {
      for(u32 j = 0; j < WINDOWS_PER_WORKSPACE; j++) projection[monitor][j] = i;
      geom.height += BORDER_SIZE * 2;
      geom.width += BORDER_SIZE * 2;
      configure_window(workspace[i], geom.x, geom.y, geom.width, geom.height,
                       0);
      break;
    } else {
      configure_window(workspace[i], geom.x, geom.y, geom.width, geom.height,
                       BORDER_SIZE);
    }
  }
}

static u32 focused_workspace(void) {
  return visible_workspaces[focused_monitor];
}

static i32 focused_window(void) { return focused_windows[focused_workspace()]; }

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

void restore_focus(void) {
  const u32 current_workspace = focused_workspace();
  const i32 current_window = focused_windows[current_workspace];
  if(current_window != -1 &&
     projection[focused_monitor][current_window] != WINDOWS_PER_WORKSPACE) {
    focus_window(workspaces[current_workspace]
                           [projection[focused_monitor][current_window]]);
    return;
  }
  focus_monitor(focused_monitor);
}

void map_request(u32 window) {
  if(focused_monitor > monitor_count) return;
  const u32 current_workspace = focused_workspace();
  u32 *const workspace = workspaces[current_workspace];
  const u32 index =
    (monitors[focused_monitor].height > monitors[focused_monitor].width)
      ? (!workspace[0]   ? 0
         : !workspace[2] ? 2
         : !workspace[3] ? 3
         : !workspace[1] ? 1
                         : 4)
      : (!workspace[0]   ? 0
         : !workspace[1] ? 1
         : !workspace[3] ? 3
         : !workspace[2] ? 2
                         : 4);
  if(index > 3) {
    if(minimized_window_count == MINIMIZE_QUEUE_SIZE) return;
    minimized_window_count++;
    for(u32 i = 0; i < minimized_window_count - 1; i++)
      minimized_windows[i + 1] = minimized_windows[i];
    minimized_windows[0] = window;
    send_minimized_windows(minimized_windows, minimized_window_count);
    return;
  }
  workspace[index] = window;
  listen_to_events(window);
  reconfigure_monitor(focused_monitor);
  send_workspace(workspace, current_workspace);
  map_window(window);
  focus_window(window);
}

void unmap_notify(u32 window) {
  const u32 current_workspace = focused_workspace();
  const i32 current_window =
    focused_windows[current_workspace] == -1
      ? -1
      : projection[focused_monitor][focused_windows[current_workspace]];
  u32 *workspace;
  for(u32 j = 0; j < monitor_count; j++) {
    workspace = workspaces[visible_workspaces[j]];
    for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
      if(workspace[i] == window) {
        workspace[i] = 0;
        reconfigure_monitor(focused_monitor);
        send_workspace(workspace, current_workspace);
        if(visible_workspaces[j] == current_workspace &&
           projection[focused_monitor][i] != current_window) {
          restore_focus();
        }
        break;
      }
    }
  }
}

void destroy_notify(u32 window) {
  for(u32 i = 0; i < WORKSPACE_COUNT; i++) {
    for(u32 j = 0; j < WINDOWS_PER_WORKSPACE; j++) {
      if(workspaces[i][j] == window) {
        workspaces[i][j] = 0;
        send_workspace(workspaces[i], i);
        return;
      }
    }
  }
}

void reset_layout_state(void) {
  for(u32 i = 0; i < WORKSPACE_COUNT; i++) {
    for(u32 j = 0; j < WINDOWS_PER_WORKSPACE; j++) workspaces[i][j] = 0;
    focused_windows[i] = -1;
  }
  for(u32 i = 0; i < MAX_MONITOR_COUNT; i++) visible_workspaces[i] = i;
}

void focus_in_notify(u32 window) {
  const u32 workspace_num = focused_workspace();
  const u32 *const restrict workspace = workspaces[workspace_num];
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    if(window == workspace[i]) {
      focused_windows[workspace_num] = projection[focused_monitor][i];
      send_focused_window(window);
      send_focused_windows((u32 *)focused_windows);
      send_focused_workspace(visible_workspaces[focused_monitor]);
      change_window_border_color(window, BORDER_FOCUSED);
      return;
    }
  }
  for(u32 j = 0; j < monitor_count; j++) {
    for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
      if(window == workspaces[visible_workspaces[j]][i]) {
        focused_monitor = j;
        send_focused_monitor(j);
        focused_windows[j] = projection[j][i];
        send_focused_window(window);
        send_focused_windows((u32 *)focused_windows);
        send_focused_workspace(visible_workspaces[j]);
        change_window_border_color(window, BORDER_FOCUSED);
        return;
      }
    }
  }
}

void focus_out_notify(u32 window) {
  change_window_border_color(window, BORDER_UNFOCUSED);
  focused_windows[focused_workspace()] = -1;
}

void focus_window_to_left(void) {
  const i32 curr_window = focused_windows[focused_workspace()];
  if(curr_window == -1 || curr_window % 2 == 0 ||
     projection[focused_monitor][curr_window] ==
       projection[focused_monitor][curr_window - 1]) {
    if(!monitors[focused_monitor].has_to_left) return;
    const u32 left = monitors[focused_monitor].to_left;
    const u32 index = (curr_window == -1)    ? 1
                      : curr_window % 2 == 1 ? curr_window
                                             : curr_window + 1;
    if(projection[left][index] != WINDOWS_PER_WORKSPACE) {
      focus_window(
        workspaces[visible_workspaces[left]][projection[left][index]]);
    } else {
      focus_monitor(left);
    }
  } else if(curr_window % 2) {
    focus_window(workspaces[focused_workspace()]
                           [projection[focused_monitor][curr_window - 1]]);
  }
}

void focus_window_to_right(void) {
  const i32 curr_window = focused_windows[focused_workspace()];
  if(curr_window == -1 || curr_window % 2 ||
     projection[focused_monitor][curr_window] ==
       projection[focused_monitor][curr_window + 1]) {
    if(!monitors[focused_monitor].has_to_right) return;
    const u32 right = monitors[focused_monitor].to_right;
    const u32 index = (curr_window == -1)    ? 0
                      : curr_window % 2 == 0 ? curr_window
                                             : curr_window - 1;
    if(projection[right][index] != WINDOWS_PER_WORKSPACE) {
      focus_window(
        workspaces[visible_workspaces[right]][projection[right][index]]);
    } else {
      focus_monitor(right);
    }
  } else if(curr_window % 2 == 0) {
    focus_window(workspaces[focused_workspace()]
                           [projection[focused_monitor][curr_window + 1]]);
  }
}

void focus_window_above(void) {
  const i32 curr_window = focused_windows[focused_workspace()];
  if(curr_window == -1 || curr_window / 2 == 0 ||
     projection[focused_monitor][curr_window] ==
       projection[focused_monitor][curr_window - 2]) {
    if(!monitors[focused_monitor].has_above) return;
    const u32 above = monitors[focused_monitor].above;
    const u32 index = (curr_window == -1)    ? 2
                      : curr_window / 2 == 0 ? curr_window
                                             : curr_window - 2;
    if(projection[above][index] != WINDOWS_PER_WORKSPACE) {
      focus_window(
        workspaces[visible_workspaces[above]][projection[above][index]]);
    } else {
      focus_monitor(monitors[focused_monitor].above);
    }
  } else if(curr_window / 2) {
    focus_window(workspaces[focused_workspace()]
                           [projection[focused_monitor][curr_window - 2]]);
  }
}

void focus_window_below(void) {
  const i32 curr_window = focused_windows[focused_workspace()];
  if(curr_window == -1 || curr_window / 2 ||
     projection[focused_monitor][curr_window] ==
       projection[focused_monitor][curr_window + 2]) {
    if(!monitors[focused_monitor].has_below) return;
    const u32 below = monitors[focused_monitor].below;
    const u32 index = (curr_window == -1)    ? 0
                      : curr_window / 2 == 1 ? curr_window
                                             : curr_window + 2;
    if(projection[below][index] != WINDOWS_PER_WORKSPACE) {
      focus_window(
        workspaces[visible_workspaces[below]][projection[below][index]]);
    } else {
      focus_monitor(monitors[focused_monitor].below);
    }
  } else if(curr_window / 2 == 0) {
    focus_window(workspaces[focused_workspace()]
                           [projection[focused_monitor][curr_window + 2]]);
  }
}

void delete_focused_window(void) {
  const i32 curr_window = focused_windows[focused_workspace()];
  if(curr_window == -1) return;
  delete_window(workspaces[focused_workspace()][curr_window]);
}

void swap_focused_window_with_left(void) {
  const u32 curr_workspace = focused_workspace();
  const i32 curr_window = focused_windows[curr_workspace];
  if(curr_window == -1 || curr_window % 2 == 0 ||
     projection[focused_monitor][curr_window] ==
       projection[focused_monitor][curr_window - 1]) {
    if(!monitors[focused_monitor].has_to_left) return;
    const u32 left = monitors[focused_monitor].to_left;
    const u32 index =
      projection[left][(curr_window == -1)    ? 0
                       : curr_window % 2 == 1 ? curr_window
                                              : curr_window + 1];
    const u32 curr_index = projection[focused_monitor][curr_window];
    if(index != WINDOWS_PER_WORKSPACE) {
      const u32 t = workspaces[curr_workspace][curr_window];
      workspaces[curr_workspace][curr_index] =
        workspaces[visible_workspaces[left]][index];
      workspaces[visible_workspaces[left]][index] = t;
      focused_windows[visible_workspaces[left]] = index;
    } else {
      workspaces[visible_workspaces[left]][0] =
        workspaces[curr_workspace][curr_index];
      workspaces[curr_workspace][curr_index] = 0;
      focused_windows[visible_workspaces[left]] = 0;
    }
    send_workspace(workspaces[curr_workspace], curr_workspace);
    send_workspace(workspaces[visible_workspaces[left]],
                   visible_workspaces[left]);
    reconfigure_monitor(focused_monitor);
    reconfigure_monitor(left);
    focused_monitor = left;
    send_focused_monitor(focused_monitor);
  } else {
    const u32 t = workspaces[curr_workspace][curr_window];
    workspaces[curr_workspace][curr_window] =
      workspaces[curr_workspace][curr_window - 1];
    workspaces[curr_workspace][curr_window - 1] = t;
    focused_windows[curr_workspace] -= 1;
    send_workspace(workspaces[curr_workspace], curr_workspace);
    reconfigure_monitor(focused_monitor);
  }
}

void swap_focused_window_with_right(void) {
  const u32 curr_workspace = focused_workspace();
  const i32 curr_window = focused_windows[curr_workspace];
  if(curr_window == -1 || curr_window % 2 == 1 ||
     projection[focused_monitor][curr_window] ==
       projection[focused_monitor][curr_window + 1]) {
    if(!monitors[focused_monitor].has_to_right) return;
    const u32 right = monitors[focused_monitor].to_right;
    const u32 index =
      projection[right][(curr_window == -1)    ? 0
                        : curr_window % 2 == 0 ? curr_window
                                               : curr_window - 1];
    const u32 curr_index = projection[focused_monitor][curr_window];
    if(index != WINDOWS_PER_WORKSPACE) {
      const u32 t = workspaces[curr_workspace][curr_window];
      workspaces[curr_workspace][curr_index] =
        workspaces[visible_workspaces[right]][index];
      workspaces[visible_workspaces[right]][index] = t;
      focused_windows[visible_workspaces[right]] = index;
    } else {
      workspaces[visible_workspaces[right]][0] =
        workspaces[curr_workspace][curr_index];
      workspaces[curr_workspace][curr_index] = 0;
      focused_windows[visible_workspaces[right]] = 0;
    }
    send_workspace(workspaces[curr_workspace], curr_workspace);
    send_workspace(workspaces[visible_workspaces[right]],
                   visible_workspaces[right]);
    reconfigure_monitor(focused_monitor);
    reconfigure_monitor(right);
    focused_monitor = right;
    send_focused_monitor(focused_monitor);
  } else {
    const u32 t = workspaces[curr_workspace][curr_window];
    workspaces[curr_workspace][curr_window] =
      workspaces[curr_workspace][curr_window + 1];
    workspaces[curr_workspace][curr_window + 1] = t;
    focused_windows[curr_workspace] += 1;
    send_workspace(workspaces[curr_workspace], curr_workspace);
    reconfigure_monitor(focused_monitor);
  }
}

void swap_focused_window_with_above(void) {
  const u32 curr_workspace = focused_workspace();
  const i32 curr_window = focused_windows[curr_workspace];
  if(curr_window == -1 || curr_window / 2 == 0 ||
     projection[focused_monitor][curr_window] ==
       projection[focused_monitor][curr_window - 2]) {
    if(!monitors[focused_monitor].has_above) return;
    const u32 above = monitors[focused_monitor].above;
    const u32 index =
      projection[above][(curr_window == -1)    ? 0
                        : curr_window / 2 == 1 ? curr_window
                                               : curr_window + 2];
    const u32 curr_index = projection[focused_monitor][curr_window];
    if(index != WINDOWS_PER_WORKSPACE) {
      const u32 t = workspaces[curr_workspace][curr_window];
      workspaces[curr_workspace][curr_index] =
        workspaces[visible_workspaces[above]][index];
      workspaces[visible_workspaces[above]][index] = t;
      focused_windows[visible_workspaces[above]] = index;
    } else {
      workspaces[visible_workspaces[above]][0] =
        workspaces[curr_workspace][curr_index];
      workspaces[curr_workspace][curr_index] = 0;
      focused_windows[visible_workspaces[above]] = 0;
    }
    send_workspace(workspaces[curr_workspace], curr_workspace);
    send_workspace(workspaces[visible_workspaces[above]],
                   visible_workspaces[above]);
    reconfigure_monitor(focused_monitor);
    reconfigure_monitor(above);
    focused_monitor = above;
    send_focused_monitor(focused_monitor);
  } else {
    const u32 t = workspaces[curr_workspace][curr_window];
    workspaces[curr_workspace][curr_window] =
      workspaces[curr_workspace][curr_window - 2];
    workspaces[curr_workspace][curr_window - 2] = t;
    focused_windows[curr_workspace] -= 2;
    send_workspace(workspaces[curr_workspace], curr_workspace);
    reconfigure_monitor(focused_monitor);
  }
}

void swap_focused_window_with_below(void) {
  const u32 curr_workspace = focused_workspace();
  const i32 curr_window = focused_windows[curr_workspace];
  if(curr_window == -1 || curr_window / 2 == 1 ||
     projection[focused_monitor][curr_window] ==
       projection[focused_monitor][curr_window + 2]) {
    if(!monitors[focused_monitor].has_below) return;
    const u32 below = monitors[focused_monitor].below;
    const u32 index =
      projection[below][(curr_window == -1)    ? 0
                        : curr_window / 2 == 0 ? curr_window
                                               : curr_window - 2];
    const u32 curr_index = projection[focused_monitor][curr_window];
    if(index != WINDOWS_PER_WORKSPACE) {
      const u32 t = workspaces[curr_workspace][curr_window];
      workspaces[curr_workspace][curr_index] =
        workspaces[visible_workspaces[below]][index];
      workspaces[visible_workspaces[below]][index] = t;
      focused_windows[visible_workspaces[below]] = index;
    } else {
      workspaces[visible_workspaces[below]][0] =
        workspaces[curr_workspace][curr_index];
      workspaces[curr_workspace][curr_index] = 0;
      focused_windows[visible_workspaces[below]] = 0;
    }
    send_workspace(workspaces[curr_workspace], curr_workspace);
    send_workspace(workspaces[visible_workspaces[below]],
                   visible_workspaces[below]);
    reconfigure_monitor(focused_monitor);
    reconfigure_monitor(below);
    focused_monitor = below;
    send_focused_monitor(focused_monitor);
  } else {
    const u32 t = workspaces[curr_workspace][curr_window];
    workspaces[curr_workspace][curr_window] =
      workspaces[curr_workspace][curr_window + 2];
    workspaces[curr_workspace][curr_window + 2] = t;
    focused_windows[curr_workspace] += 2;
    send_workspace(workspaces[curr_workspace], curr_workspace);
    reconfigure_monitor(focused_monitor);
  }
}

void change_workspace(u32 w) {
  u32 focused = focused_workspace();
  if(focused == w) return;
  for(u32 i = 0; i < monitor_count; i++) {
    fflush(stdout);
    if(w == visible_workspaces[i]) {
      visible_workspaces[focused_monitor] = visible_workspaces[i];
      visible_workspaces[i] = focused;
      send_visible_workspaces(visible_workspaces, monitor_count);
      reconfigure_monitor(focused_monitor);
      reconfigure_monitor(i);
      restore_focus();
      return;
    }
  }
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++)
    unmap_window(workspaces[focused][i]);
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    if(workspaces[w][i]) map_window(workspaces[w][i]);
  }
  visible_workspaces[focused_monitor] = w;
  send_visible_workspaces(visible_workspaces, monitor_count);
  reconfigure_monitor(focused_monitor);
  restore_focus();
}

void minimize_focused_window(void) {
  const u32 focused_work = focused_workspace();
  const i32 focused_win =
    focused_windows[focused_work] == -1
      ? -1
      : projection[focused_monitor][focused_windows[focused_work]];
  if(minimized_window_count == MINIMIZE_QUEUE_SIZE || focused_win == -1) return;
  minimized_window_count++;
  for(u32 i = 0; i < minimized_window_count - 1; i++)
    minimized_windows[i + 1] = minimized_windows[i];
  minimized_windows[0] = workspaces[focused_work][focused_win];
  send_minimized_windows(minimized_windows, minimized_window_count);
  unmap_window(workspaces[focused_work][focused_win]);
  workspaces[focused_work][focused_win] = 0;
  send_workspace(workspaces[focused_work], focused_work);
  reconfigure_monitor(focused_monitor);
  restore_focus();
}

void unminimize_window(u32 index) {
  u32 full = 1;
  if(minimized_window_count == 0) return;
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    if(workspaces[focused_workspace()][i] == 0) {
      full = 0;
      break;
    }
  }
  if(full) return;
  if(index >= minimized_window_count) index = minimized_window_count - 1;
  u32 window = minimized_windows[index];
  for(u32 i = index; i < minimized_window_count - 1; i++)
    minimized_windows[i] = minimized_windows[i + 1];
  minimized_window_count--;
  send_minimized_windows(minimized_windows, minimized_window_count);
  map_request(window);
}

void init_layout(const struct geometry *geoms, u32 m_count) {
  u32 *restrict workspace;
  init_monitors(geoms, m_count);

  send_workspace_count(WORKSPACE_COUNT);
  focused_monitor = query_focused_monitor();
  send_focused_monitor(focused_monitor);
  query_visible_workspaces(visible_workspaces, m_count);
  send_visible_workspaces(visible_workspaces, m_count);
  query_workspaces((u32 *)workspaces);
  for(u32 i = 0; i < WORKSPACE_COUNT; i++) send_workspace(workspaces[i], i);
  query_focused_windows((u32 *)focused_windows);
  send_focused_windows((u32 *)focused_windows);
  minimized_window_count = query_minimized_window_count();
  minimized_window_count = MIN(minimized_window_count, MINIMIZE_QUEUE_SIZE);
  query_minimized_windows(minimized_windows, minimized_window_count);
  send_minimized_windows(minimized_windows, minimized_window_count);

  for(u32 j = 0; j < monitor_count; j++) {
    workspace = workspaces[visible_workspaces[j]];
    for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
      listen_to_events(workspace[i]);
      map_window(workspace[i]);
    }
    reconfigure_monitor(j);
  }
}

void clean_layout_state(void) {
  delete_sent_layout_data();
  for(u32 i = 0; i < WORKSPACE_COUNT; i++) {
    for(u32 j = 0; j < WINDOWS_PER_WORKSPACE; j++) {
      if(workspaces[i][j]) delete_window(workspaces[i][j]);
    }
  }
  for(u32 i = 0; i < minimized_window_count; i++)
    delete_window(minimized_windows[i]);
}

u32 is_workspace_empty(void) {
  return focused_monitor > monitor_count ||
         projection[focused_monitor][0] == WINDOWS_PER_WORKSPACE;
}
