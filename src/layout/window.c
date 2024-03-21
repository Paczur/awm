#include "window.h"
#include "layout_types.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MIN(x,y) (((x)<=(y))?(x):(y))

static xcb_connection_t *conn;
static char *(*classes)[2];
static size_t classes_length;
static xcb_get_property_reply_t* (*get_class)(xcb_window_t, size_t);
window_list_t *windows_minimized;
window_t *windows;
pthread_rwlock_t window_lock = PTHREAD_RWLOCK_INITIALIZER;

//TODO: ref counting on global storage
static void window_set_name(window_t *window) {
  xcb_get_property_reply_t *reply = NULL;
  size_t length[2];
  char *class;
  pthread_rwlock_wrlock(&window_lock);
  window->name = calloc(WINDOW_NAME_MAX_LENGTH, sizeof(char));
  reply = get_class(window->id, WINDOW_NAME_MAX_LENGTH);
  class = xcb_get_property_value(reply);
  length[1] = xcb_get_property_value_length(reply);
  if(length[1] == 0) {
    strcpy(window->name, "?");
    goto window_set_name_cleanup;
  }
  length[0] = strnlen(class, length[1])+1;
  length[1] -= length[0] + 1;
  for(size_t i=0; i < classes_length; i++) {
    if(!strcmp(class, classes[i][0]) || !strcmp(class+length[0], classes[i][0])) {
      strcpy(window->name, classes[i][1]);
      goto window_set_name_cleanup;
    }
  }
  memcpy(window->name, class+length[0], MIN(WINDOW_NAME_MAX_LENGTH,length[1]));
  if(window->name[WINDOW_NAME_MAX_LENGTH-1] != 0)
    memcpy(window->name+(WINDOW_NAME_MAX_LENGTH-4), "...", sizeof("..."));
  if(window->name[0] == 0)
    memcpy(window->name, "?", sizeof("?"));
window_set_name_cleanup:
  pthread_rwlock_unlock(&window_lock);
  free(reply);
}

window_t* window_find(xcb_window_t w) {
  pthread_rwlock_rdlock(&window_lock);
  window_t *win = windows;
  while(win != NULL) {
    if(win->id == w) {
      pthread_rwlock_unlock(&window_lock);
      return win;
    }
    win = win->next;
  }
  pthread_rwlock_unlock(&window_lock);
  return NULL;
}

window_t *window_minimized_nth(size_t n) {
  pthread_rwlock_rdlock(&window_lock);
  window_list_t *min = windows_minimized;
  if(min == NULL) {
    pthread_rwlock_unlock(&window_lock);
    return NULL;
  }
  if(min->next == NULL || n == 0) {
    pthread_rwlock_unlock(&window_lock);
    return (min == NULL) ? NULL : min->window;
  }
  for(size_t i=0; i<n; i++) {
    if(min->next == NULL) {
      pthread_rwlock_unlock(&window_lock);
      return min->window;
    }
    min = min->next;
  }
  pthread_rwlock_unlock(&window_lock);
  return min->window;
}


bool window_set_urgency(window_t *window, bool state) {
  pthread_rwlock_wrlock(&window_lock);
  if(window->urgent != state) {
    window->urgent = state;
    pthread_rwlock_unlock(&window_lock);
    return true;
  }
  pthread_rwlock_unlock(&window_lock);
  return false;
}

bool window_set_input(window_t *window, bool state) {
  pthread_rwlock_wrlock(&window_lock);
  if(window->input != state) {
    window->input = state;
    pthread_rwlock_unlock(&window_lock);
    return true;
  }
  pthread_rwlock_unlock(&window_lock);
  return false;
}

void window_show(const window_t *window) {
  window_list_t *next;
  pthread_rwlock_wrlock(&window_lock);
  window_list_t *list = windows_minimized;
  if(list == NULL) {
    pthread_rwlock_unlock(&window_lock);
    return;
  }

  if(list->window == window) {
    next = list->next;
    free(windows_minimized);
    windows_minimized = next;
  } else if(list->next != NULL) {
    while(list->next != NULL &&
          list->next->window != window) list = list->next;
    if(list->next == NULL) {
      pthread_rwlock_unlock(&window_lock);
      return;
    }
    next = list->next->next;
    free(list->next);
    list->next = next;
  }
  pthread_rwlock_unlock(&window_lock);
}

//unmapping done before this function call
void window_minimize(window_t *window) {
  window_list_t *min;
  pthread_rwlock_wrlock(&window_lock);
  window->state = WINDOW_ICONIC;
  window->minimize = true;
  min = malloc(sizeof(window_list_t));
  min->next = windows_minimized;
  windows_minimized = min;
  min->window = window;
  pthread_rwlock_unlock(&window_lock);
  if(window->name == NULL)
    window_set_name(window);
}


void window_init(xcb_connection_t *c, const char *const(*names)[2],
                 size_t names_length,
                 xcb_get_property_reply_t *(*gc)(xcb_window_t, size_t)) {
  size_t len;
  conn = c;
  get_class = gc;
  classes = malloc(names_length*sizeof(char*[2]));
  for(size_t i=0; i<names_length; i++) {
    len = strlen(names[i][0])+1;
    classes[i][0] = malloc(len);
    strncpy(classes[i][0], names[i][0], len);
    len = strlen(names[i][1])+1;
    classes[i][1] = malloc(len);
    strncpy(classes[i][1], names[i][1], len);
  }
  classes_length = names_length;
}

void window_deinit(void) {
  window_t *window = windows;
  pthread_rwlock_wrlock(&window_lock);
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
  pthread_rwlock_unlock(&window_lock);
  for(size_t i=0; i<classes_length; i++) {
    free(classes[i][1]);
    free(classes[i][0]);
  }
  free(classes);
}


WINDOW_STATE window_event_destroy(xcb_window_t window, window_t **wp) {
  window_t *w = window_find(window);
  window_list_t *t;
  WINDOW_STATE state;
  pthread_rwlock_wrlock(&window_lock);
  window_list_t *wlist = windows_minimized;
  if(w == NULL) {
    pthread_rwlock_unlock(&window_lock);
    return -3;
  }
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
  state = w->state;

  if(state == -1) {
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
  pthread_rwlock_unlock(&window_lock);
  return state;
}

void window_event_create(xcb_window_t window) {
  pthread_rwlock_wrlock(&window_lock);
  window_t *w = malloc(sizeof(window_t));
  if(windows != NULL)
    windows->prev = w;
  w->id = window;
  w->name = NULL;
  w->state = WINDOW_WITHDRAWN;
  w->next = windows;
  w->urgent = false;
  w->prev = NULL;
  w->minimize = false;
  w->input = false;
  windows = w;
  pthread_rwlock_unlock(&window_lock);
}
