#ifndef H_GLOBAL
#define H_GLOBAL

#include <stddef.h>
#include <xcb/xcb.h>

#define LENGTH(x) (sizeof(x)/sizeof(x[0]))

typedef enum {
  MODE_NORMAL,
  MODE_INSERT
} MODE;

void sh(char*);

//Freed after some time
extern xcb_keysym_t* keysyms;
extern xcb_get_keyboard_mapping_reply_t *kmapping;

//WM
extern void (**shortcut_lookup) (void);
extern size_t shortcut_lookup_offset;
extern size_t shortcut_lookup_l;
extern MODE mode;
extern xcb_keycode_t normal_code;

//XCB
extern xcb_connection_t* conn;
extern const xcb_setup_t* setup;
extern xcb_screen_t* screen;

#endif
