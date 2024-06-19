#include "workspace.h"

#include <stdio.h>
#include <stdlib.h>

#include "grid.h"
#include "window.h"
#include "workarea.h"

workspace_t workspaces[MAX_WORKSPACES];
size_t workspace_focused;

static xcb_connection_t *conn;

workspace_t *workspace_focusedw(void) { return workspaces + workspace_focused; }

bool workspace_area_empty(size_t n, size_t m) {
  return !workspaces[n].grid[m * CELLS_PER_WORKAREA].window;
}

size_t workspace_window_count(size_t n) {
  size_t counter = 0;
  workspace_t *workspace = workspaces + n;
  for(size_t i = 0; i < CELLS_PER_WORKAREA * workarea_count; i++) {
    if(workspace->grid[i].origin == i) counter++;
  }
  return counter;
}

bool workspace_empty(size_t n) {
  for(size_t i = 0; i < workarea_count; i++) {
    if(!workspace_area_empty(n, i)) return false;
  }
  return true;
}

bool workspace_urgent(size_t n) {
  for(size_t i = 0; i < CELLS_PER_WORKAREA * workarea_count; i++) {
    if(workspaces[n].grid[i].window != NULL &&
       workspaces[n].grid[i].window->urgent)
      return true;
  }
  return false;
}

bool workspace_area_fullscreen_toggle(size_t n, size_t m) {
  bool empty = workspace_area_empty(n, m);
  bool state = workspaces[n].fullscreen[m] ^ 1;
  if(!empty || (empty && !state)) {
    workspaces[n].fullscreen[m] = state;
  }
  return workspaces[n].fullscreen[m];
}

bool workspace_area_fullscreen_set(size_t n, size_t m, bool state) {
  bool empty = workspace_area_empty(n, m);
  if(!empty || (empty && !state)) {
    workspaces[n].fullscreen[m] = state;
  }
  return workspaces[n].fullscreen[m];
}

void workspace_switch(size_t n) {
  if(n > MAX_WORKSPACES) return;
  if(n == workspace_focused) return;
  workspace_t *old = workspaces + workspace_focused;
  workspace_t *new = workspaces + n;

  for(size_t i = 0; i < workarea_count * CELLS_PER_WORKAREA; i++) {
    if(workspaces[workspace_focused].grid[i].window != NULL &&
       workspaces[workspace_focused].grid[i].origin == i) {
      xcb_unmap_window(conn, workspaces[workspace_focused].grid[i].window->id);
    }
  }
  workspace_focused = n;
  for(size_t i = 0; i < workarea_count * CELLS_PER_WORKAREA; i++) {
    if(workspaces[workspace_focused].grid[i].window != NULL &&
       workspaces[workspace_focused].grid[i].origin == i) {
      xcb_map_window(conn, workspaces[n].grid[i].window->id);
    }
  }
  grid_clean();
  grid_focus_restore();
  for(size_t i = 0; i < workarea_count; i++) {
    if(workspaces[n].update[i]) {
      grid_update(i);
      workspaces[n].update[i] = false;
    } else {
      grid_refresh();
    }
  }

#define PRINT         \
  OUT_WORKSPACE(old); \
  OUT_WORKSPACE(new);
  LOGF(LAYOUT_WORKSPACE_TRACE);
#undef PRINT
}

void workspace_area_update(size_t n, size_t m) {
  workspaces[n].update[m] = true;
}

void workspace_update(size_t n) {
  for(size_t i = 0; i < workarea_count; i++) workspaces[n].update[i] = true;
}

void workspace_init(xcb_connection_t *c) {
  conn = c;
  for(size_t i = 0; i < MAX_WORKSPACES; i++) {
    workspaces[i].grid =
      calloc(CELLS_PER_WORKAREA * workarea_count, sizeof(grid_cell_t));
    workspaces[i].cross = calloc(GRID_AXIS * workarea_count, sizeof(int));
    for(size_t j = 0; j < CELLS_PER_WORKAREA * workarea_count; j++) {
      workspaces[i].grid[j].origin = -1;
    }
    workspaces[i].update = calloc(workarea_count, sizeof(bool));
    workspaces[i].fullscreen = calloc(workarea_count, sizeof(bool));
    workspaces[i].focus = -1;
  }
}

void workspace_deinit(void) {
  for(size_t i = 0; i < MAX_WORKSPACES; i++) {
    free(workspaces[i].grid);
    free(workspaces[i].cross);
    free(workspaces[i].update);
    free(workspaces[i].fullscreen);
  }
}

void workspace_event_unmap(const window_t *win, WINDOW_STATE prev) {
  (void)win;
  if(prev < 0) return;
  for(size_t i = 0; i < workarea_count; i++) {
    if(workspace_area_empty(prev, i)) {
      workspace_area_fullscreen_set(prev, i, false);
    }
  }
}
