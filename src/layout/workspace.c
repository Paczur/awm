#include "workspace.h"
#include "workarea.h"
#include "window.h"
#include "grid.h"
#include <stdlib.h>
#include <stdio.h>

workspace_t workspaces[MAX_WORKSPACES];
size_t workspace_focused;

static xcb_connection_t *conn;

workspace_t *workspace_focusedw(void) {
  return workspaces+workspace_focused;
}

bool workspace_empty(size_t n) {
  for(size_t i=0; i<workarea_count; i++) {
    if(workspaces[n].grid[i*CELLS_PER_WORKAREA].window != NULL)
      return false;
  }
  return true;
}

bool workspace_urgent(size_t n) {
  for(size_t i=0; i<CELLS_PER_WORKAREA*workarea_count; i++) {
    if(workspaces[n].grid[i].window != NULL &&
       workspaces[n].grid[i].window->urgent)
      return true;
  }
  return false;
}

bool workspace_fullscreen_toggle(size_t n) {
  bool empty = workspace_empty(n);
  bool state = workspaces[n].fullscreen ^ 1;
  if(!empty || (empty && !state)) {
    workspaces[n].fullscreen = state;
  }
  return workspaces[n].fullscreen;
}

bool workspace_fullscreen_set(size_t n, bool state) {
  bool empty = workspace_empty(n);
  if(!empty || (empty && !state)) {
    workspaces[n].fullscreen = state;
  }
  return workspaces[n].fullscreen;
}

void workspace_switch(size_t n) {
  if(n == workspace_focused) return;
  for(size_t i=0; i<workarea_count*CELLS_PER_WORKAREA; i++) {
    if(workspaces[workspace_focused].grid[i].window != NULL &&
       workspaces[workspace_focused].grid[i].origin == i) {
      xcb_unmap_window(conn, workspaces[workspace_focused].grid[i].window->id);
    }
  }
  workspace_focused = n;
  for(size_t i=0; i<workarea_count*CELLS_PER_WORKAREA; i++) {
    if(workspaces[workspace_focused].grid[i].window != NULL &&
       workspaces[workspace_focused].grid[i].origin == i) {
      xcb_map_window(conn, workspaces[n].grid[i].window->id);
    }
  }
  grid_clean();
  grid_focus_restore();
  for(size_t i=0; i<workarea_count; i++) {
    if(workspaces[n].update[i]) {
      grid_update(i);
      workspaces[n].update[i] = false;
    } else {
      grid_refresh();
    }
  }
}

void workspace_update(size_t n) {
  for(size_t i=0; i<workarea_count; i++)
    workspaces[n].update[i] = true;
}


void workspace_init(xcb_connection_t *c) {
  conn = c;
  for(size_t i=0; i<MAX_WORKSPACES; i++) {
    workspaces[i].grid = calloc(CELLS_PER_WORKAREA*workarea_count, sizeof(grid_cell_t));
    workspaces[i].cross = calloc(GRID_AXIS*workarea_count, sizeof(int));
    for(size_t j=0; j<CELLS_PER_WORKAREA; j++) {
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
