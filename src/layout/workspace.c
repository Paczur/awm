#include "workspace.h"
#include "workarea.h"
#include "window.h"
#include "grid.h"
#include <stdlib.h>

workspace_t workspaces[MAX_WORKSPACES];
size_t workspace_focused;

static xcb_connection_t *conn;

workspace_t *workspace_focusedw(void) {
  return workspaces+workspace_focused;
}

bool workspace_empty(size_t n) {
  for(size_t i=0; i<workarea_count; i++) {
    if(workspaces[n].grid[i*CELLS_PER_MONITOR].window != NULL)
      return false;
  }
  return true;
}


void workspace_switch(size_t n) {
  size_t old_focus;
  if(n == workspace_focused) return;
  for(size_t i=0; i<workarea_count*4; i++) {
    if(workspaces[n].grid[i].window != NULL &&
       workspaces[n].grid[i].origin == i) {
      xcb_map_window(conn, workspaces[n].grid[i].window->id);
    }
  }
  old_focus = workspace_focused;
  workspace_focused = n;
  for(size_t i=0; i<workarea_count*4; i++) {
    if(workspaces[old_focus].grid[i].window != NULL &&
       workspaces[old_focus].grid[i].origin == i) {
      xcb_unmap_window(conn, workspaces[old_focus].grid[i].window->id);
    }
  }
  for(size_t i=0; i<workarea_count; i++) {
    if(workspaces[n].update[i]) {
      grid_update(i);
      workspaces[n].update[i] = false;
    } else {
      grid_refresh();
    }
  }
  if(workspaces[n].focus < workarea_count*4 &&
     workspaces[n].grid[workspaces[n].focus].window != NULL) {
    xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                        workspaces[n].grid[workspaces[n].focus].window->id,
                        XCB_CURRENT_TIME);
  }
}


void workspace_init(xcb_connection_t *c) {
  conn = c;
  for(size_t i=0; i<MAX_WORKSPACES; i++) {
    workspaces[i].grid = calloc(CELLS_PER_MONITOR*workarea_count, sizeof(grid_cell_t));
    workspaces[i].cross = calloc(GRID_AXIS*workarea_count, sizeof(int));
    for(size_t j=0; j<CELLS_PER_MONITOR; j++) {
      workspaces[i].grid[j].origin = -1;
    }
    workspaces[i].update = calloc(workarea_count, sizeof(bool));
  }
}

void workspace_deinit(void) {
  for(size_t i=0; i<MAX_WORKSPACES; i++) {
    free(workspaces[i].grid);
    free(workspaces[i].cross);
    free(workspaces[i].update);
  }
}
