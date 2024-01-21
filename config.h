#ifndef H_CONFIG
#define H_CONFIG
#define XK_LATIN1 //letters
#define XK_MISCELLANY //modifiers and special

#include <xcb/xcb.h>
#include <X11/keysymdef.h>
#include <X11/XF86keysym.h>

typedef struct {
  xcb_keysym_t keysym;
  void (*function) (void);
} shortcut_t;
void config_parse(void);

void normal_mode(void);
void insert_mode(void);
void terminal(void);
void librewolf(void);

extern size_t spawn_order[];
extern uint32_t gaps;

#endif
