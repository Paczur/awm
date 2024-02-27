#ifndef H_SHORTCUT
#define H_SHORTCUT

#include <stdbool.h>
#include <stdint.h>
#include <xcb/xcb.h>

typedef struct shortcut_t {
  struct shortcut_t *next;
  uint16_t mod_mask;
  void (*function) (void);
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
  shortcut_t *by_type[SH_TYPE_LENGTH];
} shortcut_node_t;

typedef struct shorcuts_t {
  shortcut_node_t **values;
  size_t offset;
  size_t length;
} shortcuts_t;

typedef struct shortcuts_unused_t {
  struct shortcuts_unused_t *next;
  struct shortcuts_unused_t *prev;
  shortcut_node_t* shortcut;
} shortcuts_unused_t;

extern shortcuts_t shortcuts; //[shortcut_node_t]
extern shortcuts_unused_t *shortcuts_unused; //(shortcut_node_t)

void shortcuts_print(void);
bool shortcut_handle(xcb_keycode_t, SHORTCUT_TYPE, uint16_t);
void shortcut_deinit(void);
void shortcuts_shrink(void);
void shortcut_enable(xcb_connection_t*, const xcb_screen_t*, SHORTCUT_TYPE);
void shortcut_new(const xcb_get_keyboard_mapping_reply_t*, size_t, size_t,
                  SHORTCUT_TYPE, xcb_keysym_t, uint16_t, void(*)(void));
void shortcuts_update(xcb_get_keyboard_mapping_reply_t*, size_t, size_t);
void shortcut_init(size_t, size_t);
#endif

