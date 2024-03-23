#ifndef H_LAYOUT_WORKSPACE
#define H_LAYOUT_WORKSPACE

#include "layout_types.h"
#include <xcb/xcb.h>
#include <stddef.h>
#include <stdbool.h>

extern workspace_t workspaces[MAX_WORKSPACES];
extern size_t workspace_focused;

bool workspace_fullscreen_toggle(size_t);
bool workspace_fullscreen_set(size_t, bool);
workspace_t *workspace_focusedw(void);
bool workspace_empty(size_t);
bool workspace_urgent(size_t);
void workspace_update(size_t);

void workspace_switch(size_t);

void workspace_init(xcb_connection_t*);
void workspace_deinit(void);

#endif
