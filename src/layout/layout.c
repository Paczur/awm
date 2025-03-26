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
  u8 has[5];
  u8 to[5];
};

static u32 workspaces[WORKSPACE_COUNT][WINDOWS_PER_WORKSPACE];
static u8 projection[MAX_MONITOR_COUNT][WINDOWS_PER_WORKSPACE];
static u32 visible_workspaces[MAX_MONITOR_COUNT];
static struct monitor normal[MAX_MONITOR_COUNT] = {0};
static struct geometry fullscreen[MAX_MONITOR_COUNT] = {0};
static i32 focused_windows[WORKSPACE_COUNT] = {-1, -1, -1, -1, -1,
                                               -1, -1, -1, -1, -1};
static u32 monitor_count;
static u32 focused_monitor = -1;
static u32 minimized_windows[MINIMIZE_QUEUE_SIZE];
static u32 minimized_window_count;
static i32 window_size_offsets[WORKSPACE_COUNT][2] = {0};
static u32 fullscreen_windows[WORKSPACE_COUNT];

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
  const u32 curr_workspace = visible_workspaces[monitor];
  const u32 *const workspace = workspaces[curr_workspace];
  u8 taken[WINDOWS_PER_WORKSPACE * 2];
  u8 index;
  u32 offsets_changed = 0;
  struct geometry geom;

  if(fullscreen_windows[curr_workspace]) {
    configure_and_raise(fullscreen_windows[curr_workspace],
                        fullscreen[monitor].x, fullscreen[monitor].y,
                        fullscreen[monitor].width, fullscreen[monitor].height,
                        0);
    bar_visibility(0);
    return;
  }

  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    projection[monitor][i] = workspace[i] ? i : WINDOWS_PER_WORKSPACE;
    taken[i * 2] = workspace[i] ? 1 : 0;
    taken[i * 2 + 1] = workspace[i] ? 1 : 0;
  }
  if(normal[monitor].height > normal[monitor].width) {
    expand_width(monitor, taken, index);
    expand_height(monitor, taken, index);
  } else {
    expand_height(monitor, taken, index);
    expand_width(monitor, taken, index);
  }
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    if(taken[i * 2] == 0) continue;
    geom.x = (i % 2 == 0 || taken[i * 2] == 2)
               ? normal[monitor].x + GAP_SIZE
               : normal[monitor].x + normal[monitor].width / 2 + GAP_SIZE / 2;
    geom.width =
      ((taken[i * 2] == 2) ? normal[monitor].width - GAP_SIZE * 2
                           : normal[monitor].width / 2 - GAP_SIZE * 1.5) -
      BORDER_SIZE * 2;
    geom.y = (i / 2 == 0 || taken[i * 2 + 1] == 2)
               ? normal[monitor].y + GAP_SIZE
               : normal[monitor].y + normal[monitor].height / 2 + GAP_SIZE / 2;
    geom.height =
      ((taken[i * 2 + 1] == 2) ? normal[monitor].height - GAP_SIZE * 2
                               : normal[monitor].height / 2 - GAP_SIZE * 1.5) -
      BORDER_SIZE * 2;

    // resize check
    if(taken[i * 2] != 2) {
      if(i % 2) {
        geom.x += window_size_offsets[curr_workspace][1];
        geom.width -= window_size_offsets[curr_workspace][1];
      } else {
        geom.width += window_size_offsets[curr_workspace][1];
      }
    } else if(window_size_offsets[curr_workspace][1]) {
      window_size_offsets[curr_workspace][1] = 0;
      offsets_changed = 1;
    }
    if(taken[i * 2 + 1] != 2) {
      if(i / 2) {
        geom.y += window_size_offsets[curr_workspace][0];
        geom.height -= window_size_offsets[curr_workspace][0];
      } else {
        geom.height += window_size_offsets[curr_workspace][0];
      }
    } else if(window_size_offsets[curr_workspace][0]) {
      window_size_offsets[curr_workspace][0] = 0;
      offsets_changed = 1;
    }

    // remove border on only window
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
  bar_visibility(1);
  if(offsets_changed) send_size_offsets((i32 *)window_size_offsets);
}

static u32 focused_workspace(void) {
  return visible_workspaces[focused_monitor];
}

static i32 focused_window(void) { return focused_windows[focused_workspace()]; }

static void init_monitors(const struct geometry *restrict norm,
                          const struct geometry *restrict full, u32 m_count) {
  monitor_count = m_count;
  for(u32 i = 0; i < m_count; i++) {
    normal[i].x = norm[i].x;
    normal[i].y = norm[i].y;
    normal[i].width = norm[i].width;
    normal[i].height = norm[i].height;
    fullscreen[i].x = full[i].x;
    fullscreen[i].y = full[i].y;
    fullscreen[i].width = full[i].width;
    fullscreen[i].height = full[i].height;
  }
  for(u32 i = 0; i < m_count; i++) {
    for(u32 j = 0; j < m_count; j++) {
      if(i == j) continue;
      if(normal[i].y == normal[j].y + normal[j].height) {
        normal[i].to[ABOVE] = j;
        normal[j].to[BELOW] = i;
        normal[i].has[ABOVE] = 1;
        normal[j].has[BELOW] = 1;
      } else if(normal[i].y + normal[i].height == normal[j].y) {
        normal[i].to[BELOW] = j;
        normal[j].to[ABOVE] = i;
        normal[i].has[BELOW] = 1;
        normal[j].has[ABOVE] = 1;
      }
      if(normal[i].x == normal[j].x + normal[j].width) {
        normal[i].to[LEFT] = j;
        normal[j].to[RIGHT] = i;
        normal[i].has[LEFT] = 1;
        normal[j].has[RIGHT] = 1;
      } else if(normal[i].x + normal[i].width == normal[j].x) {
        normal[i].to[RIGHT] = j;
        normal[j].to[LEFT] = i;
        normal[i].has[RIGHT] = 1;
        normal[j].has[LEFT] = 1;
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
  if(current_window >= 0 &&
     projection[focused_monitor][current_window] != WINDOWS_PER_WORKSPACE) {
    focus_window(workspaces[current_workspace]
                           [projection[focused_monitor][current_window]]);
    return;
  }
  if(current_window < 0 && projection[focused_monitor][-current_window - 1] !=
                             WINDOWS_PER_WORKSPACE) {
    focus_window(workspaces[current_workspace]
                           [projection[focused_monitor][-current_window - 1]]);
    return;
  }
  if(projection[focused_monitor][0] != WINDOWS_PER_WORKSPACE) {
    focus_window(workspaces[current_workspace][projection[focused_monitor][0]]);
    return;
  }
  focus_monitor(focused_monitor);
}

void map_request(u32 window) {
  u32 state;
  if(focused_monitor > monitor_count) return;
  const u32 current_workspace = focused_workspace();
  u32 *const workspace = workspaces[current_workspace];
  const u32 index =
    (normal[focused_monitor].height > normal[focused_monitor].width)
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
  for(u32 i = 0; i < WORKSPACE_COUNT; i++) {
    for(u32 j = 0; j < WINDOWS_PER_WORKSPACE; j++) {
      if(workspaces[i][j] == window) {
        workspaces[i][j] = 0;
        send_workspace(workspaces[i], i);
      }
    }
  }
  workspace[index] = window;
  listen_to_events(window);
  state = requested_window_state(window);
  if(state & WINDOW_STATE_FULLSCREEN) {
    fullscreen_windows[current_workspace] = window;
    set_window_fullscreen(window);
    send_fullscreen_windows(fullscreen_windows);
  }
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
    if(fullscreen_windows[visible_workspaces[j]] == window) {
      fullscreen_windows[visible_workspaces[j]] = 0;
      send_fullscreen_windows(fullscreen_windows);
    }
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
  u32 monitor;
  u32 window_i;
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    if(window == workspace[i]) {
      monitor = focused_monitor;
      window_i = i;
      goto finish;
    }
  }
  for(u32 j = 0; j < monitor_count; j++) {
    for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
      if(window == workspaces[visible_workspaces[j]][i]) {
        focused_monitor = j;
        send_focused_monitor(j);
        monitor = j;
        window_i = i;
        goto finish;
      }
    }
  }
finish:
  focused_windows[visible_workspaces[monitor]] = projection[monitor][window_i];
  send_focused_window(window);
  send_focused_windows((u32 *)focused_windows);
  send_focused_workspace(visible_workspaces[monitor]);
  change_window_border_color(window, BORDER_FOCUSED);
}

void focus_out_notify(u32 window) {
  const u32 work = focused_workspace();
  change_window_border_color(window, BORDER_UNFOCUSED);
  focused_windows[work] = -focused_windows[work] - 1;
  send_unfocused_window(window);
}

void focus_window_direction(u32 direction) {
  const i32 curr_window = focused_windows[focused_workspace()];
  const i32 offset_direction = direction - 2;
  const u32 dir_axis = ABS(offset_direction);
  const u32 end_by_axis = dir_axis == 2 ? curr_window / 2 : curr_window % 2;
  const u32 is_end = offset_direction < 0 ? !end_by_axis : end_by_axis;
  if(curr_window < 0 || is_end ||
     projection[focused_monitor][curr_window] ==
       projection[focused_monitor][curr_window + offset_direction]) {
    if(!normal[focused_monitor].has[direction]) return;
    const u32 target = normal[focused_monitor].to[direction];
    const u32 index = curr_window < 0 ? 0
                      : is_end        ? (u32)curr_window - offset_direction
                                      : (u32)curr_window;
    if(projection[target][index] != WINDOWS_PER_WORKSPACE) {
      focus_window(
        workspaces[visible_workspaces[target]][projection[target][index]]);
    } else {
      focus_monitor(target);
    }
  } else if(!is_end) {
    focus_window(
      workspaces[focused_workspace()]
                [projection[focused_monitor][curr_window + offset_direction]]);
  }
}

void close_focused_window(void) {
  const i32 curr_window = focused_windows[focused_workspace()];
  if(curr_window < 0) return;
  delete_window(workspaces[focused_workspace()][curr_window]);
}

void close_window(u32 window) { delete_window(window); }

void swap_windows_by_index(u32 n) {
  const u32 curr_workspace = focused_workspace();
  const i32 curr_window = focused_windows[curr_workspace];
  if(curr_window < 0 || n > 3 || n == (u32)curr_window) return;
  const u32 temp = workspaces[curr_workspace][n];
  workspaces[curr_workspace][n] = workspaces[curr_workspace][curr_window];
  workspaces[curr_workspace][curr_window] = temp;
  send_workspace(workspaces[curr_workspace], curr_workspace);
  reconfigure_monitor(focused_monitor);
  focused_windows[curr_workspace] = n;
}

void swap_focused_window_with_direction(u32 direction) {
  const u32 curr_workspace = focused_workspace();
  const i32 curr_window = focused_windows[focused_workspace()];
  const i32 offset_direction = direction - 2;
  const u32 dir_axis = ABS(offset_direction);
  const u32 end_by_axis = dir_axis == 2 ? curr_window / 2 : curr_window % 2;
  const u32 is_end = offset_direction < 0 ? !end_by_axis : end_by_axis;
  if(curr_window < 0 || is_end ||
     projection[focused_monitor][curr_window] ==
       projection[focused_monitor][curr_window + offset_direction]) {
    if(!normal[focused_monitor].has[direction]) return;
    const u32 target = normal[focused_monitor].to[direction];
    const u32 index =
      projection[target][curr_window < 0 ? 0
                         : is_end        ? (u32)curr_window - offset_direction
                                         : (u32)curr_window];
    const u32 curr_index = projection[focused_monitor][curr_window];
    if(index != WINDOWS_PER_WORKSPACE) {
      const u32 t = workspaces[curr_workspace][curr_window];
      workspaces[curr_workspace][curr_index] =
        workspaces[visible_workspaces[target]][index];
      workspaces[visible_workspaces[target]][index] = t;
      focused_windows[visible_workspaces[target]] = index;
    } else {
      workspaces[visible_workspaces[target]][0] =
        workspaces[curr_workspace][curr_index];
      workspaces[curr_workspace][curr_index] = 0;
      focused_windows[visible_workspaces[target]] = 0;
    }
    send_workspace(workspaces[curr_workspace], curr_workspace);
    send_workspace(workspaces[visible_workspaces[target]],
                   visible_workspaces[target]);
    reconfigure_monitor(focused_monitor);
    reconfigure_monitor(target);
    focused_monitor = target;
    send_focused_monitor(focused_monitor);
  } else {
    const u32 t = workspaces[curr_workspace][curr_window];
    workspaces[curr_workspace][curr_window] =
      workspaces[curr_workspace][curr_window + offset_direction];
    workspaces[curr_workspace][curr_window + offset_direction] = t;
    focused_windows[curr_workspace] += offset_direction;
    send_workspace(workspaces[curr_workspace], curr_workspace);
    reconfigure_monitor(focused_monitor);
  }
}

void change_size_offset(i32 x, i32 y) {
  const u32 curr_workspace = focused_workspace();
  window_size_offsets[curr_workspace][0] += x;
  window_size_offsets[curr_workspace][1] += y;
  reconfigure_monitor(focused_monitor);
  send_size_offsets((i32 *)window_size_offsets);
}

void reset_size_offset(void) {
  const u32 curr_workspace = focused_workspace();
  window_size_offsets[curr_workspace][0] = 0;
  window_size_offsets[curr_workspace][1] = 0;
  reconfigure_monitor(focused_monitor);
  send_size_offsets((i32 *)window_size_offsets);
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

void set_minimized_window(u32 window, u32 state) {
  if(state && minimized_window_count < MINIMIZE_QUEUE_SIZE) {
    for(u32 i = 0; i < WORKSPACE_COUNT; i++) {
      for(u32 j = 0; j < WINDOWS_PER_WORKSPACE; j++) {
        if(workspaces[i][j] == window) {
          for(u32 k = minimized_window_count; k > 0; k--)
            minimized_windows[k] = minimized_windows[k - 1];
          minimized_window_count++;
          minimized_windows[0] = window;
          send_minimized_windows(minimized_windows, minimized_window_count);
          set_window_minimized(window);
          workspaces[i][j] = 0;
          send_workspace(workspaces[i], i);
          for(u32 k = 0; k < monitor_count; k++) {
            if(visible_workspaces[k] == i) {
              unmap_window(window);
              reconfigure_monitor(focused_monitor);
              restore_focus();
              return;
            }
          }
        }
      }
    }
  }
  if(state != 1 && minimized_window_count > 0) {
    for(u32 i = 0; i < minimized_window_count; i++) {
      if(minimized_windows[i] == window) {
        unminimize_window(i);
        return;
      }
    }
  }
}

void minimize_focused_window(void) {
  const u32 focused_work = focused_workspace();
  const i32 focused_win =
    focused_windows[focused_work] < 0
      ? focused_windows[focused_work]
      : projection[focused_monitor][focused_windows[focused_work]];
  if(minimized_window_count == MINIMIZE_QUEUE_SIZE || focused_win < 0) return;
  for(u32 i = minimized_window_count; i > 0; i--)
    minimized_windows[i] = minimized_windows[i - 1];
  minimized_window_count++;
  minimized_windows[0] = workspaces[focused_work][focused_win];
  send_minimized_windows(minimized_windows, minimized_window_count);
  set_window_minimized(workspaces[focused_work][focused_win]);
  unmap_window(workspaces[focused_work][focused_win]);
  workspaces[focused_work][focused_win] = 0;
  send_workspace(workspaces[focused_work], focused_work);
  reconfigure_monitor(focused_monitor);
  restore_focus();
}

void unminimize_window(u32 index) {
  u32 full = 1;
  u32 workspace = focused_workspace();
  if(minimized_window_count == 0) return;
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    if(workspaces[workspace][i] == 0) {
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
  reset_window_minimized(window);
}

void init_layout(const struct geometry *norm, const struct geometry *full,
                 u32 m_count) {
  u32 *restrict workspace;
  init_monitors(norm, full, m_count);

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
  query_size_offsets((i32 *)window_size_offsets);
  send_size_offsets((i32 *)window_size_offsets);
  query_fullscreen_windows(fullscreen_windows);
  send_fullscreen_windows(fullscreen_windows);

  for(u32 i = 0; i < WORKSPACE_COUNT; i++) {
    for(u32 j = 0; j < WINDOWS_PER_WORKSPACE; j++) {
      if(workspaces[i][j] > 0) listen_to_events(workspaces[i][j]);
    }
  }

  for(u32 j = 0; j < monitor_count; j++) {
    workspace = workspaces[visible_workspaces[j]];
    for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
      map_window(workspace[i]);
    }
    reconfigure_monitor(j);
  }
  setup_root();
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

void set_fullscreen_window(u32 window, u32 state) {
  u32 workspace;
  u32 window_i;
  for(u32 i = 0; i < WORKSPACE_COUNT; i++) {
    for(u32 j = 0; j < WINDOWS_PER_WORKSPACE; j++) {
      if(workspaces[i][j] == window) {
        workspace = i;
        window_i = j;
        goto found_workspace;
      }
    }
  }
  return;
found_workspace:
  if(state != 1 && fullscreen_windows[workspace]) {
    reset_window_fullscreen(fullscreen_windows[workspace]);
    fullscreen_windows[workspace] = 0;
  } else if(state &&
            fullscreen_windows[workspace] != workspaces[workspace][window_i]) {
    fullscreen_windows[workspace] = workspaces[workspace][window_i];
    set_window_fullscreen(fullscreen_windows[workspace]);
  } else {
    return;
  }
  send_fullscreen_windows(fullscreen_windows);
  for(u32 i = 0; i < monitor_count; i++) {
    if(visible_workspaces[i] == workspace) {
      reconfigure_monitor(i);
      return;
    }
  }
}

void toggle_fullscreen_on_focused_window(void) {
  const u32 focused_work = focused_workspace();
  const i32 focused_win =
    focused_windows[focused_work] < 0
      ? focused_windows[focused_work]
      : projection[focused_monitor][focused_windows[focused_work]];
  if(focused_win < 0) return;
  if(fullscreen_windows[focused_work]) {
    reset_window_fullscreen(fullscreen_windows[focused_work]);
    fullscreen_windows[focused_work] = 0;
  } else {
    fullscreen_windows[focused_work] = workspaces[focused_work][focused_win];
    set_window_fullscreen(fullscreen_windows[focused_work]);
  }
  send_fullscreen_windows(fullscreen_windows);
  reconfigure_monitor(focused_monitor);
}
