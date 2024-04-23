#ifndef H_KEYBOARD
#define H_KEYBOARD

#include <stdbool.h>
#include <stdint.h>
#include <xcb/xcb.h>
#include <xcb/xkb.h>

typedef struct shortcut_t {
  struct shortcut_t *next;
  uint32_t state;
  void (*function)(void);
  bool repeatable;
} shortcut_t;

typedef enum SHORTCUT_TYPE {
  SH_TYPE_INSERT_MODE,
  SH_TYPE_NORMAL_MODE,
  SH_TYPE_NORMAL_MODE_RELEASE,
  SH_TYPE_LAUNCHER,
  SH_TYPE_LENGTH
} SHORTCUT_TYPE;

typedef struct shortcut_node_t {
  xcb_keysym_t keysym;
  shortcut_t *shortcuts;
} shortcut_node_t;

void shortcuts_print(void);
bool shortcut_handle(xcb_keycode_t, SHORTCUT_TYPE, uint16_t);
void shortcut_enable(const xcb_screen_t *, SHORTCUT_TYPE);
void shortcut_add(xcb_keysym_t, SHORTCUT_TYPE, uint16_t, void (*)(void), bool);
void shortcut_event_state(const xcb_xkb_state_notify_event_t *);
int shortcut_utf8(xcb_keycode_t, char *, size_t);
void shortcut_init(xcb_connection_t *);
void shortcut_deinit(void);
#endif
