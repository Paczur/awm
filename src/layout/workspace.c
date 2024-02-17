#include "workspace.h"
#include "monitor.h"
#include "../global.h"

workspace_t workspaces[MAX_WORKSPACES];
size_t workspace_focused;

workspace_t *workspace_focusedw(void) {
  return workspaces+workspace_focused;
}


void workspace_switch(size_t n) {
  size_t old_focus;
  if(n == workspace_focused) return;
  for(size_t i=0; i<monitor_count*4; i++) {
    if(workspaces[n].grid[i].window != NULL &&
       workspaces[n].grid[i].origin == i) {
      xcb_map_window(conn, workspaces[n].grid[i].window->id);
    }
  }
  old_focus = workspace_focused;
  workspace_focused = n;
  for(size_t i=0; i<monitor_count*4; i++) {
    if(workspaces[old_focus].grid[i].window != NULL &&
       workspaces[old_focus].grid[i].origin == i) {
      xcb_unmap_window(conn, workspaces[old_focus].grid[i].window->id);
    }
  }
  for(size_t i=0; i<monitor_count; i++) {
    if(workspaces[n].update[i]) {
      grid_update(i);
      workspaces[n].update[i] = false;
    } else {
      grid_refresh();
    }
  }
  if(workspaces[n].focus < monitor_count*4 &&
     workspaces[n].grid[workspaces[n].focus].window != NULL) {
    xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                        workspaces[n].grid[workspaces[n].focus].window->id,
                        XCB_CURRENT_TIME);
  }
}


void workspace_init(void) {
  for(size_t i=0; i<MAX_WORKSPACES; i++) {
    workspaces[i].grid = calloc(CELLS_PER_MONITOR*monitor_count, sizeof(grid_cell_t));
    workspaces[i].cross = calloc(GRID_AXIS*monitor_count, sizeof(int));
    for(size_t j=0; j<CELLS_PER_MONITOR; j++) {
      workspaces[i].grid[j].origin = -1;
    }
    workspaces[i].update = calloc(monitor_count, sizeof(bool));
  }
}

void workspace_deinit(void) {
  for(size_t i=0; i<MAX_WORKSPACES; i++) {
    free(workspaces[i].grid);
    free(workspaces[i].cross);
    free(workspaces[i].update);
  }
}
