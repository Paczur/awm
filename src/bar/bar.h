#ifndef H_BAR
#define H_BAR

#include <stdbool.h>

#include "bar_types.h"

size_t bar_get_containers(const bar_containers_t **);
void bar_launcher_show(void);
void bar_launcher_hide(void);
bool bar_launcher_window(xcb_window_t);
void bar_launcher_append(const char *, size_t);
void bar_launcher_erase(void);
void bar_launcher_select_left(void);
void bar_launcher_select_right(void);
char *bar_launcher_return(void);

void bar_update_minimized(void);
void bar_update_workspace(size_t);
void bar_update_mode(MODE);
void bar_update_info_highlight(int, int);
void bar_update_info(int);
void bar_visibility(size_t, bool);

void bar_redraw(xcb_window_t);

void bar_color(size_t);
void bar_focus(xcb_window_t win);

void bar_init(const bar_init_t *);
void bar_deinit(void);

#endif
