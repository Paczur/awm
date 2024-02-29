#ifndef H_LAYOUT_WORKSPACE
#define H_LAYOUT_WORKSPACE

#include "layout_types.h"
#include <xcb/xcb.h>
#include <stddef.h>
#include <stdbool.h>

#define MAX_WORKSPACES 10

extern workspace_t workspaces[MAX_WORKSPACES];
extern size_t workspace_focused;

bool workspace_fullscreen(size_t);
workspace_t *workspace_focusedw(void);
bool workspace_empty(size_t);
void workspace_update(size_t);

void workspace_switch(size_t);

void workspace_init(xcb_connection_t*);
void workspace_deinit(void);

#endif
