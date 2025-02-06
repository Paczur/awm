#include "layout.h"

#include <string.h>

#include "layout_x.h"

u32 workspaces[WORKSPACE_COUNT][WINDOWS_PER_WORKSPACE];
u32 current_workspace;
struct geometry monitors[MAX_MONITOR_COUNT];

static void reconfigure_workspace(u32 w) {
  const u32 *const workspace = workspaces[w];
  const u32 window_count =
    !!workspace[0] + !!workspace[1] + !!workspace[2] + !!workspace[3];
  const struct geometry *const monitor = monitors;
  const u32 x = monitor->x;
  const u32 y = monitor->y;
  const u32 width = monitor->width;
  const u32 height = monitor->height;
  if(window_count == 1) {
    const u32 index =
      workspace[0]
        ? 0
        : (workspace[1] ? 1 : (workspace[2] ? 2 : (workspace[3] ? 3 : 0)));
    configure_window(workspace[index], x, y, monitors[0].width,
                     monitors[0].height);
  } else if(window_count == 4) {
    configure_window(workspace[0], x, y, width / 2, height / 2);
    configure_window(workspace[1], x + width / 2, y, width / 2, height / 2);
    configure_window(workspace[2], x, y + height / 2, width / 2, height / 2);
    configure_window(workspace[3], x + width / 2, y + height / 2, width / 2,
                     height / 2);
  } else if(window_count == 2) {
    if(!!workspace[0] == !!workspace[2]) {  // one above other
      const u32 offset = workspace[0] ? 0 : 1;
      configure_window(workspace[0 + offset], x, y, width, height / 2);
      configure_window(workspace[2 + offset], x, y + height / 2, width,
                       height / 2);
    } else {
      const u32 indexes[2] = {workspace[0] ? 0 : 2, workspace[1] ? 1 : 3};
      configure_window(workspace[indexes[0]], x, y, width / 2, height);
      configure_window(workspace[indexes[1]], x + width / 2, y, width / 2,
                       height);
    }
  } else if(window_count == 3) {
    if(!!workspace[0] != !!workspace[2]) {  // left is slice
      if(workspace[0]) {
        configure_window(workspace[0], x, y, width / 2, height);
        configure_window(workspace[1], x + width / 2, y, width / 2, height / 2);
        configure_window(workspace[3], x + width / 2, y + height / 2, width / 2,
                         height / 2);
      } else {
        configure_window(workspace[1], x + width / 2, y, width / 2, height / 2);
        configure_window(workspace[2], x, y, width / 2, height);
        configure_window(workspace[3], x + width / 2, y + height / 2, width / 2,
                         height / 2);
      }
    } else {  // right is slice
      if(workspace[1]) {
        configure_window(workspace[0], x, y, width / 2, height / 2);
        configure_window(workspace[1], x + width / 2, y, width / 2, height);
        configure_window(workspace[2], x, y + height / 2, width / 2,
                         height / 2);
      } else {
        configure_window(workspace[0], x, y, width / 2, height / 2);
        configure_window(workspace[2], x + width / 2, y, width / 2, height / 2);
        configure_window(workspace[3], x + width / 2, y + height / 2, width / 2,
                         height);
      }
    }
  }
}

void init_layout(const struct geometry *geoms, u32 monitor_count) {
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
