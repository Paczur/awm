#include "grid.h"
#include "workspace.h"
#include "workarea.h"
#include <xcb/xcb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define X(pos) ((pos)%HOR_CELLS_PER_WORKAREA)
#define Y(pos) ((pos)%CELLS_PER_WORKAREA/VERT_CELLS_PER_WORKAREA)
#define COMB(x, y) ((x)+(y)*VERT_CELLS_PER_WORKAREA)

#define CONF_ATOMS 5

static uint32_t *confstate;
static size_t *spawn_order;
static size_t spawn_order_len;
static size_t gap_size;
static size_t border_size;
static xcb_connection_t *conn;

bool grid_pos_invalid(size_t n) { return n >= workarea_count*CELLS_PER_WORKAREA; }
size_t grid_mon2pos(size_t m) { return m*CELLS_PER_WORKAREA; }

static grid_cell_t *grid_focusedc(void) {
  workspace_t *w = workspaces+workspace_focused;
  return w->grid+w->focus;
}

static grid_cell_t *grid_pos2cellwo(size_t n, size_t wo) {
  return grid_pos_invalid(n) ? NULL : workspaces[wo].grid+n;
}

static grid_cell_t *grid_pos2cell(size_t n) {
  return grid_pos_invalid(n) ? NULL : workspace_focusedw()->grid+n;
}

static size_t grid_pos2originwo(size_t n, size_t wo) {
  const workspace_t *workspace = workspaces+wo;
  if(grid_pos_invalid(n) ||
     grid_pos_invalid(workspace->grid[n].origin))
    return -1;
  return workspace->grid[n].origin;
}

static size_t grid_pos2origin(size_t n) {
  const workspace_t *workspace = workspace_focusedw();
  if(grid_pos_invalid(n) ||
     grid_pos_invalid(workspace->grid[n].origin))
    return -1;
  return workspace->grid[n].origin;
}

static grid_cell_t *grid_pos2originc(size_t n) {
  const workspace_t *workspace = workspace_focusedw();
  if(grid_pos_invalid(n) ||
     grid_pos_invalid(workspace->grid[n].origin))
    return NULL;
  return workspace->grid+workspace->grid[n].origin;
}

static bool grid_mon_empty(size_t m) {
  return workspace_focusedw()->grid[grid_mon2pos(m)].window == NULL;
}

static void grid_expand_horizontally(size_t m, uint32_t* values, size_t offset) {
  size_t p, ri;
  for(size_t i=0; i<CELLS_PER_WORKAREA; i++) {
    p = grid_mon2pos(m);
    ri = COMB(!X(i), Y(i));
    if(grid_pos2win(p+i) == NULL &&
       grid_pos2win(p+ri) != NULL) {
      grid_pos2cell(p+i)->window = grid_pos2win(p+ri);
      grid_pos2cell(p+i)->origin = grid_pos2origin(p+ri);

      values[offset+ri*CONF_ATOMS+2] += values[offset+i*CONF_ATOMS+2]+
        gap_size*2 + border_size*2;
      if(X(i) == 0) {
        values[offset+ri*CONF_ATOMS+0] -= values[offset+i*CONF_ATOMS+2]+
          gap_size*2;
      }
    }
  }
}

static void grid_expand_vertically(size_t m, uint32_t* values, size_t offset) {
  size_t p, ri;
  for(size_t i=0; i<CELLS_PER_WORKAREA; i++) {
    p = grid_mon2pos(m);
    ri = COMB(X(i), !Y(i));
    if(grid_pos2win(p+i) == NULL &&
       grid_pos2win(p+ri) != NULL) {
      grid_pos2cell(p+i)->window = grid_pos2win(p+ri);
      grid_pos2cell(p+i)->origin = grid_pos2origin(p+ri);

      values[offset+ri*CONF_ATOMS+3] += values[offset+i*CONF_ATOMS+3]+
        gap_size*2 + border_size*2;
      if(Y(i) == 0) {
        values[offset+ri*CONF_ATOMS+1] -= values[offset+i*CONF_ATOMS+3]+
          gap_size*2;
      }
    }
  }
}

static const workarea_t *grid_workareas(void) {
  if(workspace_focusedw()->fullscreen)
    return workareas_fullscreen;
  return workareas;
}

static void grid_calculate(size_t m, uint32_t* values, size_t offset) {
  const workspace_t* workspace = workspace_focusedw();
  size_t p;
  size_t t;
  const workarea_t *workareas = grid_workareas();
  for(size_t i=0; i<CELLS_PER_WORKAREA; i++) {
    p = grid_mon2pos(m)+i;
    if(grid_pos2origin(p) != p)
      grid_pos2cell(p)->window = NULL;

    values[offset+i*CONF_ATOMS+0] = workareas[m].x + gap_size +
      (X(i) == 0 ?
       0 :
       (workareas[m].w/2 + workspace->cross[m*GRID_AXIS+0]));
    values[offset+i*CONF_ATOMS+1] = workareas[m].y + gap_size +
      (Y(i) == 0 ?
       0 :
       (workareas[m].h/2 + workspace->cross[m*GRID_AXIS+1]));
    values[offset+i*CONF_ATOMS+2] = workareas[m].w/2 -
      gap_size*2 - border_size*2 +
      (X(i) == 0 ?
       workspace->cross[m*GRID_AXIS+0] :
       -workspace->cross[m*GRID_AXIS+0]);
    values[offset+i*CONF_ATOMS+3] = workareas[m].h/2 -
      gap_size*2 - border_size*2 +
      (Y(i) == 0 ?
       workspace->cross[m*GRID_AXIS+1] :
       -workspace->cross[m*GRID_AXIS+1]);
    values[offset+i*CONF_ATOMS+4] = border_size;
  }
  if(workareas[m].w < workareas[m].h) {
    grid_expand_horizontally(m, values, offset);
    grid_expand_vertically(m, values, offset);
  } else {
    grid_expand_vertically(m, values, offset);
    grid_expand_horizontally(m, values, offset);
  }
  if(grid_pos2mon(grid_focused()) == m) {
    t = offset+grid_focusedc()->origin%CELLS_PER_WORKAREA*CONF_ATOMS;
    values[t+0] -= gap_size;
    values[t+1] -= gap_size;
    values[t+2] += gap_size*2+border_size*2;
    values[t+3] += gap_size*2+border_size*2;
    values[t+4] = 0;
  }
#define PRINT OUT(m); OUT_ARR(values+offset, CELLS_PER_WORKAREA*CONF_ATOMS);
  LOGF(LAYOUT_GRID_TRACE);
#undef PRINT
}

static void grid_force_update(size_t pos) {
  for(size_t i=0; i<CONF_ATOMS; i++) {
    confstate[pos*CONF_ATOMS+i] = -1;
  }
}


void grid_clean(void) {
  memset(confstate, -1, CONF_ATOMS*CELLS_PER_WORKAREA*workarea_count*sizeof(uint32_t));
}

window_t *grid_pos2win(size_t n) {
  return grid_pos_invalid(n) ? NULL : workspace_focusedw()->grid[n].window;
}

size_t grid_ord2pos(size_t n) {
  size_t i;
  size_t count = 0;
  for(i=0; i<spawn_order_len; i++) {
    if(!grid_pos_invalid(spawn_order[i])) {
      count++;
      if(count == n+1)
        return spawn_order[i];
    }
  }
  return -1;
}

size_t grid_xwin2pos(xcb_window_t window) {
  for(size_t i=0; i<workarea_count*CELLS_PER_WORKAREA; i++) {
    if(grid_pos2win(i) != NULL &&
       grid_pos2win(i)->id == window)
      return i;
  }
  return -1;
}

xcb_window_t grid_pos2xwin(size_t n) {
  const window_t *w = grid_pos2win(n);
  if(w == NULL) return (xcb_window_t)-1;
  return w->id;
}

size_t grid_win2pos(const window_t *win) {
  if(!win || win->state < 0) return -1;
  const workspace_t *workspace = workspaces+win->state;
  for(size_t i=0; i<workarea_count*CELLS_PER_WORKAREA; i++) {
    if(workspace->grid[i].window == win)
      return i;
  }
  return -1;
}

window_t *grid_focusedw(void) {
  const workspace_t *workspace = workspace_focusedw();
  const grid_cell_t *cell;
  if(grid_pos_invalid(workspace->focus))
    return NULL;
  cell = workspace->grid+workspace->focus;
  return (cell == NULL) ?  NULL : cell->window;
}

size_t grid_pos2mon(size_t n) {
  return grid_pos_invalid(n) ?  SIZE_MAX : n/CELLS_PER_WORKAREA;
}

size_t grid_next_poswo(size_t wo) {
  for(size_t i=0; i<spawn_order_len; i++) {
    if(!grid_pos_invalid(spawn_order[i]) &&
       grid_pos2originwo(spawn_order[i], wo) != spawn_order[i]) {
      return spawn_order[i];
    }
  }
  return SIZE_MAX;
}

size_t grid_next_pos(void) {
  for(size_t i=0; i<spawn_order_len; i++) {
    if(!grid_pos_invalid(spawn_order[i]) &&
       grid_pos2origin(spawn_order[i]) != spawn_order[i]) {
      return spawn_order[i];
    }
  }
  return SIZE_MAX;
}

//w might be freed, can't access it
void grid_unmark(window_t *w) {
  workspace_t *workspace;
  for(size_t j=0; j<MAX_WORKSPACES; j++) {
    workspace = workspaces+j;
    for(size_t i=0; i<workarea_count*CELLS_PER_WORKAREA; i++) {
      if(workspace->grid[i].origin == i &&
         workspace->grid[i].window == w) {
        workspace->grid[workspace->grid[i].origin].window = NULL;
        workspace->grid[workspace->grid[i].origin].origin = -1;
        workspace->update[grid_pos2mon(i)] = true;
        return;
      }
    }
  }
}

void grid_refresh(void) {
  for(size_t i=0; i<workarea_count; i++) {
    grid_calculate(i, confstate, i*CONF_ATOMS*CELLS_PER_WORKAREA);
  }
}

void grid_update(size_t m) {
  const workspace_t *workspace = workspace_focusedw();
  uint32_t newstate[CONF_ATOMS*CELLS_PER_WORKAREA];
  grid_calculate(m, newstate, 0);
  if(grid_mon_empty(m)) {
    for(size_t i=0; i<GRID_AXIS; i++) {
      workspace->cross[m*GRID_AXIS+i] = 0;
    }
    memcpy(confstate+m*CONF_ATOMS*CELLS_PER_WORKAREA, newstate, sizeof(newstate));
    return;
  }
  if(grid_pos2win(COMB(0, 0)) == grid_pos2win(COMB(0, 1)) &&
     grid_pos2win(COMB(1, 0)) == grid_pos2win(COMB(1, 1))) {
    workspace->cross[m*GRID_AXIS+1] = 0;
  }
  if(grid_pos2win(COMB(0, 0)) == grid_pos2win(COMB(1, 0)) &&
     grid_pos2win(COMB(0, 1)) == grid_pos2win(COMB(1, 1))) {
    workspace->cross[m*GRID_AXIS+0] = 0;
  }
  for(size_t i=0; i<CELLS_PER_WORKAREA; i++) {
    if(grid_pos2origin(grid_mon2pos(m)+i) == grid_mon2pos(m)+i &&
       grid_pos2win(grid_mon2pos(m)+i) != NULL &&
       (confstate[m*CONF_ATOMS*CELLS_PER_WORKAREA+i*CONF_ATOMS+0]
        != newstate[i*CONF_ATOMS+0] ||
        confstate[m*CONF_ATOMS*CELLS_PER_WORKAREA+i*CONF_ATOMS+1]
        != newstate[i*CONF_ATOMS+1] ||
        confstate[m*CONF_ATOMS*CELLS_PER_WORKAREA+i*CONF_ATOMS+2]
        != newstate[i*CONF_ATOMS+2] ||
        confstate[m*CONF_ATOMS*CELLS_PER_WORKAREA+i*CONF_ATOMS+3]
        != newstate[i*CONF_ATOMS+3] ||
        confstate[m*CONF_ATOMS*CELLS_PER_WORKAREA+i*CONF_ATOMS+4]
        != newstate[i*CONF_ATOMS+4])) {
      xcb_configure_window(conn,
                           grid_pos2win(grid_mon2pos(m)+i)->id,
                           XCB_CONFIG_WINDOW_X |
                           XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
                           XCB_CONFIG_WINDOW_HEIGHT |
                           XCB_CONFIG_WINDOW_BORDER_WIDTH,
                           newstate+i*CONF_ATOMS);
    }
  }
#define PRINT OUT(m); OUT(workspace); \
  OUT_ARR(confstate+m*CONF_ATOMS*CELLS_PER_WORKAREA, CONF_ATOMS*CELLS_PER_WORKAREA); \
  OUT_ARR(newstate, CONF_ATOMS*CELLS_PER_WORKAREA);
  LOGF(LAYOUT_GRID_TRACE);
#undef PRINT
  memcpy(confstate+m*CONF_ATOMS*CELLS_PER_WORKAREA, newstate, sizeof(newstate));
}

void grid_adjust_pos(void) {
  size_t t;
  size_t t2;
  bool *moved;
  size_t count = 0;
  size_t iter = 0;
  size_t orig;
  moved = calloc(workarea_count, sizeof(bool));
  for(size_t i=0; i<CELLS_PER_WORKAREA*workarea_count; i++) {
    if(grid_pos2origin(i) == i)
      count++;
  }
  for(size_t i=0; i<spawn_order_len && iter<count; i++) {
    t = spawn_order[i];
    if(grid_pos_invalid(t))
      continue;
    orig = grid_pos2origin(t);
    iter++;
    if(grid_pos2win(t) == NULL || orig != t) {
      for(size_t j=i+1; j<spawn_order_len; j++) {
        t2 = spawn_order[j];
        if(!grid_pos_invalid(t2) &&
           grid_pos2win(t2) != NULL &&
           grid_pos2origin(t2) == t2) {
          grid_pos2cell(t)->origin = t;
          grid_pos2cell(t)->window = grid_pos2win(t2);
          grid_pos2cell(t2)->origin = -1;
          grid_pos2cell(t2)->window = NULL;
          moved[grid_pos2mon(t)] = true;
          moved[grid_pos2mon(t2)] = true;
          grid_force_update(t);
          grid_force_update(t2);
          break;
        }
      }
    }
  }
  for(size_t i=0; i<workarea_count; i++) {
    if(moved[i]) {
      grid_update(i);
    }
  }
  free(moved);
}

bool grid_focus_pick(void) {
  workspace_t *workspace = workspace_focusedw();
  window_t *win;
  for(size_t i=0; i<workarea_count; i++) {
    win = workspace->grid[i*CONF_ATOMS].window;
    if(win != NULL && win->input) {
      grid_focus(i*CONF_ATOMS);
      return true;
    }
  }
  return false;
}

bool grid_focus_restore(void) {
  grid_cell_t *cell = grid_focusedc();
  if(cell != NULL && cell->window != NULL && cell->window->input) {
    workspace_focusedw()->focus = cell->origin;
    xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                        cell->window->id, XCB_CURRENT_TIME);
    return true;
  }
  return grid_focus_pick();
}

void grid_place_windowwo(window_t *window, size_t grid_i, bool assume_map,
                         size_t wo) {
  size_t m = grid_pos2mon(grid_i);
  grid_pos2cellwo(grid_i, wo)->window = window;
  grid_pos2cellwo(grid_i, wo)->origin = grid_i;
  if(wo == workspace_focused && !assume_map) {
    grid_force_update(grid_i);
    grid_update(m);
    xcb_map_window(conn, window->id);
  } else {
    workspaces[wo].update[m] = true;
  }
}

void grid_place_window(window_t *window, size_t grid_i, bool assume_map) {
  size_t m = grid_pos2mon(grid_i);
  grid_pos2cell(grid_i)->window = window;
  grid_pos2cell(grid_i)->origin = grid_i;
  grid_force_update(grid_i);
  grid_update(m);
  if(!assume_map)
    xcb_map_window(conn, window->id);
}

size_t grid_focused(void) { return workspace_focusedw()->focus; }


bool grid_swap(size_t n1, size_t n2) {
  workspace_t *workspace = workspace_focusedw();
  window_t *window;
  size_t m1, m2;
  grid_cell_t *c1, *c2;

  c1 = grid_pos2cell(n1);
  c2 = grid_pos2cell(n2);
  if(c1 == NULL || c2 == NULL || c1->window == c2->window) return false;

  c1 = grid_pos2originc(n1);
  c2 = grid_pos2originc(n2);
  m1 = grid_pos2mon(n1);
  m2 = grid_pos2mon(n2);

  window = c1->window;
  c1->window = c2->window;
  c2->window = window;

  grid_force_update(n1);
  grid_force_update(n2);
  if(workspace->focus == n1) {
    workspace->focus = n2;
  } else if(workspace->focus == n2) {
    workspace->focus = n1;
  }
  grid_update(m1);
  if(m1 != m2) {
    grid_update(m2);
  }
  return true;
}

bool grid_focus(size_t n) {
  grid_cell_t *target = grid_pos2cell(n);
  grid_cell_t *curr = grid_focusedc();
  if(target == NULL || target->window == NULL || !target->window->input ||
     (curr != NULL && curr->window != NULL && curr->window == target->window))
    return false;
  xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                      target->window->id, XCB_CURRENT_TIME);
  return true;
}

void grid_reset_sizes(size_t m) {
  for(size_t i=0; i<2; i++) {
    workspace_focusedw()->cross[m*2+i] = 0;
  }
  grid_update(m);
}

void grid_resize_h(size_t m, int h) {
  const workspace_t* workspace = workspace_focusedw();
  size_t ph = grid_workareas()[m].h/2 - gap_size*2 - border_size*2
    - workspace->cross[m*GRID_AXIS+1];
  if((h > 0 && (ph - h > ph || ph - h == 0)) ||
     (h < 0 && (ph + workspace->cross[m*GRID_AXIS+1]*2 + h >
                ph + workspace->cross[m*GRID_AXIS+1]*2 ||
                ph + workspace->cross[m*GRID_AXIS+1]*2 + h == 0)))
    return;
  workspace->cross[m*2+1] += h;
  grid_update(m);
}

void grid_resize_w(size_t m, int w) {
  const workspace_t* workspace = workspace_focusedw();
  size_t pw = grid_workareas()[m].w/2 - gap_size*2 - border_size*2
    - workspace->cross[m*GRID_AXIS+0];
  if((w > 0 && (pw - w > pw || pw - w == 0)) ||
     (w < 0 && (pw + workspace->cross[m*GRID_AXIS+0]*2 + w >
                pw + workspace->cross[m*GRID_AXIS+0]*2 ||
                pw + workspace->cross[m*GRID_AXIS+0]*2 + w == 0)))
    return;
  workspace->cross[m*GRID_AXIS+0] += w;
  grid_update(m);
}

bool grid_show(window_t *window) {
  size_t pos = grid_next_pos();
  if(grid_pos_invalid(pos))
    return false;
  grid_place_window(window, pos, false);
  return true;
}

void grid_minimizew(const window_t *win) { xcb_unmap_window(conn, win->id); }

bool grid_minimize(size_t n) {
  grid_cell_t *cell = grid_pos2cell(n);
  if(cell == NULL || cell->window == NULL) return false;
  xcb_unmap_window(conn, cell->window->id);
  return true;
}

void grid_destroy(size_t n) {
  grid_cell_t *cell = grid_pos2cell(n);
  if(cell == NULL || cell->window == NULL) return;
  xcb_kill_client(conn, cell->window->id);
}

size_t grid_below(void) {
  size_t t = grid_focused();
  return grid_pos2mon(t)*CELLS_PER_WORKAREA+COMB(X(t), !Y(t));
}

size_t grid_above(void) { return grid_below(); }

size_t grid_to_right(void) {
  window_t *next = grid_focusedc()->window;
  size_t t = grid_focused();
  size_t i = 0;
  while(grid_focusedc()->window == next &&
        i < workarea_count*HOR_CELLS_PER_WORKAREA) {
    if(t == workarea_count*CELLS_PER_WORKAREA-1 ||
       t == workarea_count*CELLS_PER_WORKAREA-HOR_CELLS_PER_WORKAREA-1) {
      t = COMB(0, Y(t));
    } else {
      t += (X(t) == HOR_CELLS_PER_WORKAREA-1) ? (CELLS_PER_WORKAREA-1) : 1;
    }
    next = grid_pos2cell(t)->window;
    i++;
  }
  if(i == workarea_count*HOR_CELLS_PER_WORKAREA)
    return grid_focused();
  return t;
}

size_t grid_to_left(void) {
  workspace_t *workspace = workspace_focusedw();
  window_t *next = grid_focusedc()->window;
  size_t t = workspace->focus;
  size_t i = 0;
  while(grid_focusedc()->window == next &&
        i < workarea_count*HOR_CELLS_PER_WORKAREA) {
    if(t == 0 || t == HOR_CELLS_PER_WORKAREA) {
      t = COMB(1, Y(t)) + (workarea_count-1)*CELLS_PER_WORKAREA;
    } else {
      t -= X(t) == 0 ? CELLS_PER_WORKAREA-1 : HOR_CELLS_PER_WORKAREA-1;
    }
    next = grid_pos2win(t);
    i++;
  }
  if(i == workarea_count*HOR_CELLS_PER_WORKAREA)
    return grid_focused();
  return t;
}

size_t grid_focusable_below(void) {
  size_t t = grid_focused();
  size_t pos = grid_pos2mon(t)*CELLS_PER_WORKAREA+COMB(X(t), !Y(t));
  const window_t *win =
    grid_pos2win(pos);
  if(win == NULL || !win->input)
    return t;
  return pos;
}

size_t grid_focusable_above(void) { return grid_focusable_below(); }

size_t grid_focusable_to_right(void) {
  window_t *next = grid_focusedc()->window;
  size_t t = grid_focused();
  size_t i = 0;
  while((grid_focusedc()->window == next || next == NULL || next->input) &&
        i < workarea_count*HOR_CELLS_PER_WORKAREA) {
    if(t == workarea_count*CELLS_PER_WORKAREA-1 ||
       t == workarea_count*CELLS_PER_WORKAREA-HOR_CELLS_PER_WORKAREA-1) {
      t = COMB(0, Y(t));
    } else {
      t += (X(t) == HOR_CELLS_PER_WORKAREA-1) ? (CELLS_PER_WORKAREA-1) : 1;
    }
    next = grid_pos2cell(t)->window;
    i++;
  }
  if(i == workarea_count*HOR_CELLS_PER_WORKAREA)
    return grid_focused();
  return t;
}

size_t grid_focusable_to_left(void) {
  workspace_t *workspace = workspace_focusedw();
  window_t *next = grid_focusedc()->window;
  size_t t = workspace->focus;
  size_t i = 0;
  while((grid_focusedc()->window == next || next == NULL || next->input) &&
        i < workarea_count*HOR_CELLS_PER_WORKAREA) {
    if(t == 0 || t == HOR_CELLS_PER_WORKAREA) {
      t = COMB(1, Y(t)) + (workarea_count-1)*CELLS_PER_WORKAREA;
    } else {
      t -= X(t) == 0 ? CELLS_PER_WORKAREA-1 : HOR_CELLS_PER_WORKAREA-1;
    }
    next = grid_pos2win(t);
    i++;
  }
  if(i == workarea_count*HOR_CELLS_PER_WORKAREA)
    return grid_focused();
  return t;
}

bool grid_restore_window(window_t *win, size_t wo) {
  if(win->state >= WINDOW_WORKSPACE_START) return true;
  size_t next = grid_next_poswo(wo);

  if(grid_pos_invalid(next))
    return false;

  grid_place_windowwo(win, next, false, wo);
  return true;
}


void grid_init(xcb_connection_t *c, const grid_init_t *init) {
  conn = c;
  gap_size = init->gaps;
  border_size = init->borders;
  spawn_order = malloc(init->spawn_order_length*sizeof(size_t));
  spawn_order_len = init->spawn_order_length;
  memcpy(spawn_order, init->spawn_order, spawn_order_len*sizeof(size_t));
  confstate = calloc(workarea_count*CONF_ATOMS*CELLS_PER_WORKAREA, sizeof(uint32_t));
}

void grid_deinit(void) {
  free(spawn_order);
  free(confstate);
}


void grid_event_focus(xcb_window_t window) {
  size_t grid_i = grid_focused();
  size_t in = grid_xwin2pos(window);
  if(grid_pos_invalid(in)) return;
  size_t temp = grid_pos2origin(in);
  if(!grid_pos_invalid(grid_i) &&
     !grid_pos_invalid(temp) &&
     grid_pos2cell(grid_i)->window == grid_pos2cell(temp)->window)
    return;
  if(!grid_pos_invalid(grid_i) && grid_pos2cell(grid_i)->window != NULL) {
    confstate[grid_i*CONF_ATOMS+0] += gap_size;
    confstate[grid_i*CONF_ATOMS+1] += gap_size;
    confstate[grid_i*CONF_ATOMS+2] -= gap_size*2+border_size*2;
    confstate[grid_i*CONF_ATOMS+3] -= gap_size*2+border_size*2;
    confstate[grid_i*CONF_ATOMS+4] = border_size;
    xcb_configure_window(conn,
                         grid_pos2cell(grid_i)->window->id,
                         XCB_CONFIG_WINDOW_X |
                         XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
                         XCB_CONFIG_WINDOW_HEIGHT |
                         XCB_CONFIG_WINDOW_BORDER_WIDTH,
                         confstate+grid_i*CONF_ATOMS);
  }
  if(!grid_pos_invalid(temp)) {
    confstate[temp*CONF_ATOMS+0] -= gap_size;
    confstate[temp*CONF_ATOMS+1] -= gap_size;
    confstate[temp*CONF_ATOMS+2] += gap_size*2+border_size*2;
    confstate[temp*CONF_ATOMS+3] += gap_size*2+border_size*2;
    confstate[temp*CONF_ATOMS+4] = 0;
    xcb_configure_window(conn,
                         window,
                         XCB_CONFIG_WINDOW_X |
                         XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
                         XCB_CONFIG_WINDOW_HEIGHT |
                         XCB_CONFIG_WINDOW_BORDER_WIDTH,
                         confstate+temp*CONF_ATOMS);
    workspace_focusedw()->focus = temp;
  }
}

bool grid_event_map(window_t *window) {
  if(window->state >= WINDOW_WORKSPACE_START) return true;
  size_t next = grid_next_pos();

  if(grid_pos_invalid(next))
    return false;

  grid_place_window(window, next, false);
  return true;
}

void grid_event_unmap(xcb_window_t window) {
  xcb_focus_out_event_t fallback = { XCB_FOCUS_OUT, .event = window };
  size_t pos = grid_xwin2pos(window);
  if(grid_pos_invalid(pos)) return;
  pos = grid_pos2origin(pos);
  grid_pos2cell(pos)->window = NULL;
  grid_pos2cell(pos)->origin = -1;
  grid_update(grid_pos2mon(pos));
  if(grid_focused() == pos && !grid_focus_restore()) {
    xcb_send_event(conn, 0, window, XCB_EVENT_MASK_FOCUS_CHANGE,
                   (char*)&fallback);
  }
  grid_adjust_pos();
  grid_update(grid_pos2mon(pos));
}
