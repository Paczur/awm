#include "window.h"
#include "layout_structs.h"
#include <stdlib.h>

window_list_t *windows_minimized;
window_t *windows;

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
