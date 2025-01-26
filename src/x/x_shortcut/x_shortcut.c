#include "x_shortcut.h"

#include <string.h>

#include "../../shortcut/shortcut.h"
#include "../x_atom/x_atom.h"
#include "../x_private.h"

#define FROM_X_KEYMAP(keymap) ((xcb_get_keyboard_mapping_reply_t *)(keymap))
#define TO_X_KEYMAP(keymap) ((x_keymap *)(keymap))

struct x_keymap {
  xcb_get_keyboard_mapping_reply_t keymap;
};

static awm_vector_init(uint32_t) keymap_syms;
static uint32_t syms_per_code;

static uint32_t mode = 2;

uint32_t x_shortcut_mode(void) {
  xcb_get_property_cookie_t cookie;
  xcb_get_property_reply_t *reply;
  if(mode == 2) {
    cookie = xcb_get_property(conn, 0, screen->root, AWM_SHORTCUT_MODE,
                              XCB_ATOM_CARDINAL, 0, 1);
    reply = xcb_get_property_reply(conn, cookie, NULL);
    if(reply != NULL && xcb_get_property_value_length(reply) > 0) {
      mode = *(uint32_t *)xcb_get_property_value(reply);
    } else {
      x_set_shortcut_mode(SHORTCUT_MODE_NORMAL);
      if(reply) free(reply);
    }
  }
  return mode;
}

void x_set_shortcut_mode(uint32_t m) { mode = m; }

void x_ungrab_keyboard(void) {
  xcb_ungrab_key(conn, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
}

void x_grab_keyboard(void) {
  xcb_grab_key(conn, 1, screen->root, XCB_MOD_MASK_ANY, XCB_GRAB_ANY,
               XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
}

void x_grab_key(uint8_t key, uint8_t mod) {
  xcb_grab_key(conn, 1, screen->root, mod, key + setup->min_keycode,
               XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
}

void x_ungrab_key(uint8_t key, uint8_t mod) {
  xcb_ungrab_key(conn, key + setup->min_keycode, screen->root, mod);
}

void x_refresh_keymap(void) {
  xcb_generic_error_t *e = NULL;
  xcb_get_keyboard_mapping_reply_t *rep = NULL;
  xcb_get_keyboard_mapping_cookie_t cookie = xcb_get_keyboard_mapping(
  conn, setup->min_keycode, setup->max_keycode - setup->min_keycode + 1);
  rep = xcb_get_keyboard_mapping_reply(conn, cookie, &e);
  if(rep == NULL || e != NULL) {
    log(LOG_LEVEL_FATAL, "Couldn't get keymap: %u", e->error_code);
    return;
  }
  syms_per_code = rep->keysyms_per_keycode;
  awm_vector_size(&keymap_syms) = xcb_get_keyboard_mapping_keysyms_length(rep);
  memcpy(awm_vector_array(&keymap_syms), xcb_get_keyboard_mapping_keysyms(rep),
         syms_per_code * awm_vector_size(&keymap_syms) * 32);
}

uint8_t x_keymap_syms_per_code(void) {
  if(awm_vector_size(&keymap_syms) == 0) x_refresh_keymap();
  return syms_per_code;
}

uint32_t x_keymap_length(void) {
  if(awm_vector_size(&keymap_syms) == 0) x_refresh_keymap();
  return awm_vector_size(&keymap_syms);
}

uint32_t *x_keymap_syms(void) {
  if(awm_vector_size(&keymap_syms) == 0) x_refresh_keymap();
  return awm_vector_array(&keymap_syms);
}

uint8_t x_key_code(x_event *ev) {
  return ((xcb_key_press_event_t *)ev)->detail - setup->min_keycode;
}

uint8_t x_min_key_code(void) { return setup->min_keycode; }

uint8_t x_key_mod(x_event *ev) { return ((xcb_key_press_event_t *)ev)->state; }
