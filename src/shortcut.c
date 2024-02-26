#include "shortcut.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

shortcuts_t shortcuts;
shortcuts_unused_t *shortcuts_unused;

void shortcuts_unused_print(void) {
  puts("UNUSED:");
  shortcuts_unused_t *unused = shortcuts_unused;
  while(unused != NULL) {
    printf("%u ", unused->shortcut->keysym);
    unused = unused->next;
  }
  puts("");
}

void shortcuts_print(void) {
  puts("SHORTCUTS");
  for(size_t i=0; i<shortcuts.length; i++) {
    if(shortcuts.values[i] == NULL) {
      puts("NULL");
    } else {
      printf("%u\n", shortcuts.values[i]->keysym);
    }
  }
}

xcb_keycode_t keysym_to_keycode(xcb_keysym_t keysym,
                                const xcb_get_keyboard_mapping_reply_t *kmapping,
                                size_t start, size_t end) {
  xcb_keysym_t* keysyms = xcb_get_keyboard_mapping_keysyms(kmapping);
  for(size_t i=start; i<end; i++) {
    for(size_t j=0; j<kmapping->keysyms_per_keycode; j++) {
      if(keysyms[(i-start) * kmapping->keysyms_per_keycode+j] ==
         keysym)
        return i;
    }
  }
  return -1;
}

bool shortcut_handle(xcb_keycode_t keycode, SHORTCUT_TYPE type, uint16_t state) {
  shortcut_t *sh;
  size_t lookup;

  lookup = keycode - shortcuts.offset;
  if(lookup >= shortcuts.length)
    return false;
  if(shortcuts.values[lookup] == NULL) return false;
  sh = shortcuts.values[lookup]->by_type[type];
  while(sh != NULL) {
    if(state == sh->mod_mask) {
      sh->function();
      return true;
    } else {
      sh = sh->next;
    }
  }
  return false;
}

void shortcuts_shrink(void) {
  size_t start;
  size_t end;
  size_t length;
  for(start=0; shortcuts.values[start] == NULL; start++) { }
  for(end=shortcuts.length-1; shortcuts.values[end] == NULL; end--) {}
  length = end - start;
  memmove(shortcuts.values, shortcuts.values+start,
          length*sizeof(shortcut_node_t*));
  shortcuts.values = realloc(shortcuts.values, length*sizeof(shortcut_node_t*));
  shortcuts.offset += start;
  shortcuts.length = length;
}

bool shortcut_unused_add_node(shortcut_node_t *node) {
  shortcuts_unused_t *unused = shortcuts_unused;
  while(unused != NULL && unused->shortcut->keysym != node->keysym) {
    unused = unused->next;
  }
  if(unused == NULL) {
    unused = shortcuts_unused;
    shortcuts_unused = malloc(sizeof(shortcuts_unused_t));
    shortcuts_unused->next = unused;
    shortcuts_unused->prev = NULL;
    unused->prev = shortcuts_unused;
    unused = shortcuts_unused;
  }
  if(unused->shortcut == NULL) {
    unused->shortcut = node;
    return true;
  }
  return false;
}

void shortcut_unused_add(xcb_keysym_t keysym, SHORTCUT_TYPE type,
                         uint16_t mod_mask, void (*function) (void)) {
  shortcuts_unused_t *unused = shortcuts_unused;
  shortcut_t *sh;
  while(unused != NULL && unused->shortcut->keysym != keysym) {
    unused = unused->next;
  }
  if(unused == NULL) {
    unused = shortcuts_unused;
    shortcuts_unused = malloc(sizeof(shortcuts_unused_t));
    shortcuts_unused->shortcut = NULL;
    shortcuts_unused->next = unused;
    shortcuts_unused->prev = NULL;
    if(unused != NULL)
      unused->prev = shortcuts_unused;
    unused = shortcuts_unused;
  }
  if(unused->shortcut == NULL) {
    unused->shortcut = malloc(sizeof(shortcut_node_t));
    unused->shortcut->keysym = keysym;
  }
  sh = unused->shortcut->by_type[type];
  unused->shortcut->by_type[type] = malloc(sizeof(shortcut_t));
  unused->shortcut->by_type[type]->next = sh;
  unused->shortcut->by_type[type]->function = function;
  unused->shortcut->by_type[type]->mod_mask = mod_mask;
}

void shortcut_add(xcb_keysym_t keysym, xcb_keycode_t keycode,
                  SHORTCUT_TYPE type, uint16_t mod_mask,
                  void (*function) (void)) {
  shortcut_node_t *node;
  shortcut_t *sh;
  if(keycode < shortcuts.offset) {
    memmove(shortcuts.values-(keycode-shortcuts.offset),
            shortcuts.values, shortcuts.length*sizeof(shortcut_node_t*));
    shortcuts.length -= keycode-shortcuts.offset;
    shortcuts.offset += keycode-shortcuts.offset;
  } else if(keycode-shortcuts.offset >= shortcuts.length) {
    shortcuts.length = keycode-shortcuts.offset+1;
    shortcuts.values = realloc(shortcuts.values, shortcuts.length *
                               sizeof(shortcut_node_t*));
  }
  if(shortcuts.values[keycode-shortcuts.offset] == NULL) {
    shortcuts.values[keycode-shortcuts.offset] = calloc(1,sizeof(shortcut_node_t));
  }
  node = shortcuts.values[keycode-shortcuts.offset];
  node->keysym = keysym;
  sh = node->by_type[type];
  node->by_type[type] = malloc(sizeof(shortcut_t));
  node->by_type[type]->next = sh;
  node->by_type[type]->mod_mask = mod_mask;
  node->by_type[type]->function = function;
}

void shortcut_new(const xcb_get_keyboard_mapping_reply_t *kmap,
                  size_t first, size_t last,
                  SHORTCUT_TYPE type, xcb_keysym_t keysym,
                  uint16_t mod_mask, void (*function) (void)) {
  xcb_keysym_t* keysyms = xcb_get_keyboard_mapping_keysyms(kmap);
  bool found = false;
  for(size_t i=first; i<last; i++) {
    for(size_t j=0; j<kmap->keysyms_per_keycode; j++) {
      if(keysyms[(i-first) * kmap->keysyms_per_keycode+j] ==
         keysym) {
        found = true;
        shortcut_add(keysym, i, type, mod_mask, function);
      }
    }
  }
  if(!found) {
    shortcut_unused_add(keysym, type, mod_mask, function);
  }
}

void shortcuts_update(xcb_get_keyboard_mapping_reply_t *kmap,
                      size_t start, size_t end) {
  size_t common_start;
  size_t common_end;
  bool used;
  bool found;
  xcb_keysym_t *keysyms = xcb_get_keyboard_mapping_keysyms(kmap);
  shortcut_node_t *node;

  common_start = (start > shortcuts.offset) ? start : shortcuts.offset;
  common_end = (shortcuts.offset+shortcuts.length > end) ?
    end : shortcuts.offset+shortcuts.length;

  for(size_t i=common_start; i<common_end; i++) {
    found = false;
    for(size_t j=0; j<kmap->keysyms_per_keycode; j++) {
      if(shortcuts.values[i-shortcuts.offset] != NULL &&
         shortcuts.values[i-shortcuts.offset]->keysym ==
         keysyms[(i-start)*kmap->keysyms_per_keycode+j]) {
        found = true;
        break;
      }
    }
    if(!found) {
      node = shortcuts.values[i-shortcuts.offset];
      used = shortcut_unused_add_node(node);
      shortcuts.values[i-shortcuts.offset] = NULL;
      if(!used)
        free(node);
    }
  }
  //TODO: Remove shortcuts
  //TODO: Add shortcuts
 }

void shortcut_enable(xcb_connection_t *conn, const xcb_screen_t *screen,
                     SHORTCUT_TYPE type) {
  shortcut_t *sh;
  xcb_ungrab_key(conn, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
  for(size_t i=0; i<shortcuts.length; i++) {
    if(shortcuts.values[i] == NULL) continue;
    sh = shortcuts.values[i]->by_type[type];
    while(sh != NULL) {
      xcb_grab_key(conn, 1, screen->root, sh->mod_mask,
                   i+shortcuts.offset,
                   XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
      sh = sh->next;
    }
  }
}

void shortcut_init(size_t start, size_t end) {
  shortcuts.offset = start;
  shortcuts.length = end-start;
  shortcuts.values = calloc(shortcuts.length, sizeof(shortcut_node_t));
}

void shortcut_deinit(void) {
  shortcut_t *sh;
  shortcut_t *t;
  for(size_t i=0; i<shortcuts.length; i++) {
    if(shortcuts.values[i] != NULL) {
      for(size_t j=0; j<SH_TYPE_LENGTH; j++) {
        sh = shortcuts.values[i]->by_type[j];
        if(sh != NULL) {
          while(sh->next != NULL) {
            t = sh->next;
            free(sh);
            sh = t;
          }
          free(sh);
        }
      }
      free(shortcuts.values[i]);
    }
  }
  free(shortcuts.values);
  shortcuts.values = NULL;
}
