#ifndef H_BAR
#define H_BAR

#include "bar_structs.h"
#include "../layout/layout.h"
#include "../structs.h"
#include "../system.h"
#include "../config.h"
#include "../mode.h"

void bar_launcher_show(void);
void bar_launcher_hide(void);
bool bar_launcher_window(xcb_window_t);
void bar_launcher_append(const xcb_key_press_event_t*);
void bar_launcher_erase(void);
void bar_launcher_select_left(void);
void bar_launcher_select_right(void);
void bar_launcher_run(void);

void bar_update_minimized(void);
void bar_update_workspace(void);
void bar_update_mode(void);
void bar_update_info_highlight(int, int);
void bar_update_info(int);

void bar_redraw(void);

void bar_init(const rect_t*, size_t);
void bar_deinit(void);

#endif
