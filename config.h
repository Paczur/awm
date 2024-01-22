#ifndef H_CONFIG
#define H_CONFIG
#define fwindow(n) \
  void focus_window_ ## n (void)

#define XK_LATIN1 //letters
#define XK_MISCELLANY //modifiers and special

#include <xcb/xcb.h>
#include "window.h"
#include <X11/keysymdef.h>
#include <X11/XF86keysym.h>

typedef struct {
  xcb_keysym_t keysym;
  void (*function) (void);
} shortcut_t;

fwindow(0);
fwindow(1);
fwindow(2);
fwindow(3);
fwindow(4);
fwindow(5);
fwindow(6);
fwindow(7);
fwindow(8);
fwindow(9);
void config_parse(void);

void normal_mode(void);
void insert_mode(void);
void terminal(void);
void librewolf(void);
void focus_window_down(void);
void focus_window_up(void);
void focus_window_left(void);
void focus_window_right(void);
void destroy_current_window(void);

extern size_t spawn_order[24];
extern uint32_t gaps;

#endif
