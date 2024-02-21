#include "window.h"
#include "layout_structs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static xcb_connection_t *conn;
static xcb_atom_t wm_class;
static const char *const (*classes)[2];
static size_t classes_length;
window_list_t *windows_minimized;
window_t *windows;

//TODO: ref counting on global storage
//TODO: improve this function
static void window_set_name(window_t *window) {
  xcb_get_property_reply_t *reply = NULL;
  size_t length[2];
  char *class;
  window->name = calloc(WINDOW_NAME_MAX_LENGTH, sizeof(char));
  xcb_get_property_cookie_t cookie =
    xcb_get_property(conn, 0, window->id, wm_class, XCB_ATOM_STRING,
                     0, 50);
  reply = xcb_get_property_reply(conn, cookie, NULL);

  class = xcb_get_property_value(reply);
  length[1] = xcb_get_property_value_length(reply);
  length[0] = strnlen(class, length[1])+1;
  length[1] -= length[0] + 1;
  for(size_t i=0; i < classes_length; i++) {
    if(strcmp(class, classes[i][0]) == 0) {
      strcpy(window->name, classes[i][1]);
      free(reply);
      return;
    } else if(strcmp(class+length[0], classes[i][0]) == 0) {
      strcpy(window->name, classes[i][1]);
      free(reply);
      return;
    }
  }
  if(length[1] > WINDOW_NAME_MAX_LENGTH) {
    memcpy(window->name, class+length[0], WINDOW_NAME_MAX_LENGTH);
    if(window->name[WINDOW_NAME_MAX_LENGTH-1] != 0) {
       window->name[WINDOW_NAME_MAX_LENGTH-1] = 0;
       window->name[WINDOW_NAME_MAX_LENGTH-2] = '.';
       window->name[WINDOW_NAME_MAX_LENGTH-3] = '.';
       window->name[WINDOW_NAME_MAX_LENGTH-4] = '.';
    }
  } else {
    memcpy(window->name, class+length[0], length[1]);
  }
  if(window->name[0] < 'A' ||
     window->name[0] > 'z' ||
     (window->name[0] > 'Z' &&
      window->name[0] < 'a')) {
    window->name[0] = '?';
    window->name[1] = 0;
  }
  free(reply);
}

window_t* window_find(xcb_window_t w) {
  window_t *win = windows;
  while(win != NULL) {
    if(win->id == w) {
      return win;
    }
    win = win->next;
  }
  return NULL;
}

window_t *window_minimized_nth(size_t n) {
  window_list_t *min = windows_minimized;
  if(min == NULL) return NULL;
  if(min->next == NULL || n == 0) {
    return (min == NULL) ? NULL : min->window;
  }
  for(size_t i=0; i<n; i++) {
    if(min->next == NULL)
      return min->window;
    min = min->next;
  }
  return min->window;
}


void window_show(const window_t *window) {
  window_list_t *next;
  window_list_t *list = windows_minimized;
  if(list == NULL) return;

  if(list->window == window) {
    next = list->next;
    free(windows_minimized);
    windows_minimized = next;
  } else if(list->next != NULL) {
    while(list->next != NULL &&
          list->next->window != window) list = list->next;
    if(list->next == NULL) return;
    next = list->next->next;
    free(list->next);
    list->next = next;
  }
}

//unmapping done before this function call
void window_minimize(window_t *window) {
  window_list_t *min;
  window->pos = -1;
  min = malloc(sizeof(window_list_t));
  min->next = windows_minimized;
  windows_minimized = min;
  min->window = window;
  if(window->name == NULL)
    window_set_name(window);
}


void window_init(xcb_connection_t *c, const char *const(*names)[2],
                 size_t names_length) {
  conn = c;
  classes = names;
  classes_length = names_length;
  char wm_class_str[] = "WM_CLASS";
  xcb_intern_atom_reply_t *reply = NULL;
  xcb_intern_atom_cookie_t cookie;
  cookie = xcb_intern_atom(conn, 0, sizeof(wm_class_str)-1, wm_class_str);
  while(reply == NULL) {
    reply = xcb_intern_atom_reply(conn, cookie, NULL);
    if(reply == NULL) {
      cookie = xcb_intern_atom(conn, 0, sizeof(wm_class_str)-1, wm_class_str);
    }
  }
  wm_class = reply->atom;
  free(reply);
}

void window_deinit(void) {
  window_t *window = windows;
  window_t *t;
  while(window != NULL) {
    t = window;
    window = window->next;
    if(t->name != NULL)
      free(t->name);
    free(t);
  }
  windows = NULL;

  window_list_t *min = windows_minimized;
  window_list_t *temp;
  while(min != NULL) {
    temp = min;
    min = min->next;
    free(temp);
  }
  windows_minimized = NULL;
}


int window_event_destroy(xcb_window_t window, window_t **wp) {
  window_t *w = window_find(window);
  window_list_t *wlist = windows_minimized;
  window_list_t *t;
  size_t pos;
  if(w == NULL) return -3;
  if(w->prev != NULL) {
    w->prev->next = w->next;
  } else {
    windows = w->next;
  }
  if(w->next != NULL) {
    w->next->prev = w->prev;
  }
  if(w->name != NULL)
    free(w->name);
  pos = w->pos;

  if(pos == (size_t)-1) {
    if(wlist->window == w) {
      windows_minimized = wlist->next;
      free(wlist);
    } else {
      while(wlist->next->window != w)
        wlist = wlist->next;
      t = wlist->next;
      wlist->next = wlist->next->next;
      free(t);
    }
  }

  *wp = w;
  free(w);
  return pos;
}

void window_event_create(xcb_window_t window) {
  window_t *w = malloc(sizeof(window_t));
  if(windows != NULL)
    windows->prev = w;
  w->id = window;
  w->name = NULL;
  w->pos = -2;
  w->next = windows;
  w->prev = NULL;
  windows = w;
}
