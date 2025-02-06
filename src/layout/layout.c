#include "layout.h"

#include <string.h>

#include "layout_x.h"

#define POSITION_FULLSCREEN 0
#define POSITION_VERT 1
#define POSITION_HOR 2
#define POSITION_QUAR 3

static u32 workspaces[WORKSPACE_COUNT][WINDOWS_PER_WORKSPACE];
static u32 current_workspace;
static struct geometry monitors[MAX_MONITOR_COUNT];
static u32 border;

static void position(u32 win, u32 pos, u32 type) {
  const struct geometry *const monitor = monitors;
  const u32 x = monitor->x;
  const u32 y = monitor->y;
  const u32 width = monitor->width;
  const u32 height = monitor->height;
  const u32 position[4][2] = {
    {x + border, y + border},
    {x + width / 2 + border, y + border},
    {x + border, y + height / 2 + border},
    {x + width / 2 + border, y + height / 2 + border}};
  const u32 lay[4][2] = {
    {width - border * 2, height - border * 2},
    {width / 2 - border * 2, height - border * 2},
    {width - border * 2, height / 2 - border * 2},
    {width / 2 - border * 2, height / 2 - border * 2},
  };
  configure_window(win, position[pos][0], position[pos][1], lay[type][0],
                   lay[type][1], border);
}

static void reconfigure_workspace(u32 w) {
  const u32 *const workspace = workspaces[w];
  const u32 window_count =
    !!workspace[0] + !!workspace[1] + !!workspace[2] + !!workspace[3];
  if(window_count == 1) {
    const u32 index =
      workspace[0]
        ? 0
        : (workspace[1] ? 1 : (workspace[2] ? 2 : (workspace[3] ? 3 : 0)));
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

void init_layout(const struct geometry *geoms, u32 monitor_count, u32 b) {
  border = b;
  memcpy(monitors, geoms, sizeof(struct geometry) * monitor_count);
  current_workspace = query_current_workspace();
  query_workspaces((u32 *)workspaces);
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++)
    map_window(workspaces[current_workspace][i]);
}

void map_request(u32 window) {
  u32 *const workspace = workspaces[current_workspace];
  const u32 index =
    !workspace[0]
      ? 0
      : (!workspace[1] ? 1 : (!workspace[3] ? 3 : (!workspace[2] ? 2 : 4)));
  if(index > 3) return;
  workspace[index] = window;
  reconfigure_workspace(current_workspace);
  send_workspace(workspace, current_workspace);
  map_window(window);
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
}
