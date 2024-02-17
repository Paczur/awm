#ifndef H_LAYOUT_WORKSPACE
#define H_LAYOUT_WORKSPACE

#include "grid.h"
#include "../system_config.h"

typedef struct workspace_t {
  grid_cell_t *grid;
  size_t focus;
  int *cross;
  bool *update;
} workspace_t;

extern workspace_t workspaces[MAX_WORKSPACES];
extern size_t workspace_focused;

workspace_t *workspace_focusedw(void);

void workspace_switch(size_t);

void workspace_init(void);
void workspace_deinit(void);

#endif
