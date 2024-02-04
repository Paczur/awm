#ifndef H_CONFIG
#define H_CONFIG
#define fwindow(n) \
  void focus_window_ ## n (void)
#define swindow(n) \
  void swap_window_ ## n (void)

#define XK_LATIN1 //letters
#define XK_MISCELLANY //modifiers and special

#include <xcb/xcb.h>
#include "window.h"
#include <X11/keysymdef.h>
#include <X11/XF86keysym.h>

typedef enum {
  MOD_NONE,
  MOD_SHIFT,
  MOD_ALT,
  MOD_SUPER
} MODIFIER;

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
swindow(0);
swindow(1);
swindow(2);
swindow(3);
swindow(4);
swindow(5);
swindow(6);
swindow(7);
swindow(8);
swindow(9);
void config_parse(void);

void normal_mode(void);
void insert_mode(void);
void terminal(void);
void librewolf(void);
void focus_window_down(void);
void focus_window_up(void);
void focus_window_left(void);
void focus_window_right(void);
void swap_window_up(void);
void swap_window_down(void);
void swap_window_left(void);
void swap_window_right(void);
void destroy_current_window(void);
void enlarge_width(void);
void enlarge_height(void);
void shrink_width(void);
void shrink_height(void);

#endif
