#ifndef H_LAYOUT_WORKSPACE
#define H_LAYOUT_WORKSPACE

#include <stdbool.h>
#include <stddef.h>
#include <xcb/xcb.h>

#include "layout_types.h"

extern workspace_t workspaces[MAX_WORKSPACES];
extern size_t workspace_focused;

bool workspace_area_fullscreen_toggle(size_t, size_t);
bool workspace_area_fullscreen_set(size_t, size_t, bool);
workspace_t *workspace_focusedw(void);
bool workspace_area_empty(size_t, size_t);
size_t workspace_window_count(size_t);
bool workspace_empty(size_t);
bool workspace_urgent(size_t);
void workspace_area_update(size_t, size_t);
void workspace_update(size_t);
void workspace_switch(size_t);

void workspace_init(xcb_connection_t *);
void workspace_deinit(void);

void workspace_event_unmap(const window_t *, WINDOW_STATE);
#endif
