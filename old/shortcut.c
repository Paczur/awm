#include "shortcut.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xkbcommon/xkbcommon-x11.h>

#define MAX_KEYSYMS 200  // 105 is full size keyboard

static struct xkb_context *ctx;
static int32_t core_device;
static struct xkb_keymap *keymap;
static struct xkb_state *xkbstate;
static xcb_connection_t *conn;
static xcb_keycode_t last_shortcut_keycode;
static uint32_t last_shortcut_state;

static shortcut_node_t *shortcut_map[MAX_KEYSYMS] = {};

static void shortcut_free_keymap(void) {
  xkb_state_unref(xkbstate);
  xkb_keymap_unref(keymap);
}

static void shortcut_load_keymap(void) {
  keymap = xkb_x11_keymap_new_from_device(ctx, conn, core_device,
                                          XKB_KEYMAP_COMPILE_NO_FLAGS);
  xkbstate = xkb_x11_state_new_from_device(keymap, conn, core_device);
}

bool shortcut_handle(const xcb_key_press_event_t *event, SHORTCUT_TYPE type) {
  const xkb_keysym_t *syms;
  xcb_keycode_t keycode = event->detail;
  uint16_t state = event->state;

  uint32_t full_state = (type << 16) | state;
  full_state &= ~(XCB_MOD_MASK_2 | XCB_MOD_MASK_3 | XCB_MOD_MASK_5);
  shortcut_t *t;
  int num_syms;
  size_t index;

  num_syms = xkb_state_key_get_syms(xkbstate, keycode, &syms);
  for(int i = 0; i < num_syms; i++) {
    index = syms[i] % MAX_KEYSYMS;
    while(shortcut_map[index] != NULL && shortcut_map[index]->keysym != syms[i])
      index = (index + 1) % MAX_KEYSYMS;
    if(shortcut_map[index] != NULL) {
      t = shortcut_map[index]->shortcuts;
      while(t != NULL) {
        if(t->state != full_state) {
          t = t->next;
        } else
          break;
      }
      if(t != NULL && (t->repeatable || full_state != last_shortcut_state ||
                       keycode != last_shortcut_keycode)) {
        t->function();
        last_shortcut_state = full_state;
        last_shortcut_keycode = keycode;
        return true;
      }
      last_shortcut_state = full_state;
      last_shortcut_keycode = keycode;
    }
  }
  return false;
}

void shortcut_add(xcb_keysym_t keysym, SHORTCUT_TYPE type, uint16_t mod_mask,
                  void (*function)(void), bool repeatable) {
  shortcut_t *t;
  if(mod_mask & XCB_MOD_MASK_SHIFT) {
    keysym = xkb_keysym_to_upper(keysym);
  } else {
    keysym = xkb_keysym_to_lower(keysym);
  }
  size_t index = keysym % MAX_KEYSYMS;
  while(shortcut_map[index] != NULL && shortcut_map[index]->keysym != keysym)
    index = (index + 1) % MAX_KEYSYMS;
  if(shortcut_map[index] == NULL) {
    shortcut_map[index] = malloc(sizeof(shortcut_node_t));
    shortcut_map[index]->shortcuts = NULL;
    shortcut_map[index]->keysym = keysym;
  }
  t = shortcut_map[index]->shortcuts;
  shortcut_map[index]->shortcuts = malloc(sizeof(shortcut_t));
  shortcut_map[index]->shortcuts->next = t;
  t = shortcut_map[index]->shortcuts;
  t->state = (type << 16) | (mod_mask);
  t->function = function;
  t->repeatable = repeatable;
}

void shortcut_enable(const xcb_screen_t *screen, SHORTCUT_TYPE type) {
  shortcut_t *sh;
  bool found;
  xcb_mod_mask_t mask;
  int num_syms;
  xcb_mod_mask_t *used;
  const xkb_keysym_t *keysyms;
  xkb_keycode_t min = xkb_keymap_min_keycode(keymap);
  xkb_keycode_t max = xkb_keymap_max_keycode(keymap);

  xcb_ungrab_key(conn, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
  if(type == SH_TYPE_NORMAL || type == SH_TYPE_NORMAL_RELEASE) {
    xcb_grab_key(conn, 1, screen->root, XCB_MOD_MASK_ANY, XCB_GRAB_ANY,
                 XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    return;
  }
  used = malloc((max - min + 1) * sizeof(xcb_mod_mask_t));
  memset(used, (xcb_mod_mask_t)-1, sizeof(xcb_mod_mask_t) * (max - min + 1));
  for(size_t i = 0; i < MAX_KEYSYMS; i++) {
    if(shortcut_map[i] != NULL) {
      found = false;
      sh = shortcut_map[i]->shortcuts;
      mask = 0;
      while(sh != NULL) {
        if((sh->state >> 16) == type) {
          mask |= sh->state & 0xFFFF;
          found = true;
        }
        sh = sh->next;
      }
      if(found) {
        for(size_t j = min; j <= max; j++) {
          num_syms = xkb_state_key_get_syms(xkbstate, j, &keysyms);
          for(int k = 0; k < num_syms; k++) {
            if(keysyms[k] == shortcut_map[i]->keysym) {
              used[j - min] = mask;
              break;
            }
          }
        }
      }
    }
  }
  for(size_t i = 0; i <= max - min; i++) {
    if(used[i] != (xcb_mod_mask_t)-1) {
      xcb_grab_key(conn, 1, screen->root, used[i], i + min, XCB_GRAB_MODE_ASYNC,
                   XCB_GRAB_MODE_ASYNC);
    }
  }
  free(used);
}

void shortcut_event_state(const xcb_xkb_state_notify_event_t *event) {
  if(event->xkbType == XCB_XKB_STATE_NOTIFY) {
    xkb_state_update_mask(xkbstate, event->baseMods, event->latchedMods,
                          event->lockedMods, event->baseGroup,
                          event->latchedGroup, event->lockedGroup);
  } else if(event->xkbType == XCB_XKB_NEW_KEYBOARD_NOTIFY ||
            event->xkbType == XCB_XKB_MAP_NOTIFY) {
    shortcut_free_keymap();
    shortcut_load_keymap();
  }
}

int shortcut_utf8(xcb_keycode_t keycode, char *buff, size_t size) {
  return xkb_state_key_get_utf8(xkbstate, keycode, buff, size);
}

void shortcut_init(xcb_connection_t *c) {
  xcb_xkb_per_client_flags_reply_t *reply;
  xcb_xkb_per_client_flags_cookie_t cookie;
  const uint32_t mask = XCB_XKB_PER_CLIENT_FLAG_GRABS_USE_XKB_STATE |
                        XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT;
  conn = c;
  ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  core_device = xkb_x11_get_core_keyboard_device_id(conn);
  cookie = xcb_xkb_per_client_flags(conn, XCB_XKB_ID_USE_CORE_KBD, mask, mask,
                                    0, 0, 0);
  reply = xcb_xkb_per_client_flags_reply(conn, cookie, NULL);
  free(reply);
  shortcut_load_keymap();
}

void shortcut_deinit(void) {
  shortcut_free_keymap();
  xkb_context_unref(ctx);
}
