#include "workarea.h"

#include <stdlib.h>
#include <string.h>

#include "layout_types.h"

workarea_t *workareas;
workarea_t *workareas_fullscreen;
size_t workarea_count;
size_t workarea_capacity;

void workarea_init(const workarea_t *ws, const workarea_t *full,
                                 size_t count) {
  workareas = malloc(count * sizeof(workarea_t));
  workareas_fullscreen = malloc(count * sizeof(workarea_t));
  workarea_count = count;
  workarea_capacity = count;
  memcpy(workareas, ws, count * sizeof(workarea_t));
  memcpy(workareas_fullscreen, full, count * sizeof(workarea_t));
}

void workarea_update(const workarea_t *ws, const workarea_t *full,
                                   size_t count) {
  if(workarea_count != count) {
    workareas = realloc(workareas, count * sizeof(workarea_t));
    workareas_fullscreen =
      realloc(workareas_fullscreen, count * sizeof(workarea_t));
    workarea_count = count;
    if(workarea_count > workarea_capacity) workarea_capacity = workarea_count;
  }
  memcpy(workareas, ws, count * sizeof(workarea_t));
  memcpy(workareas_fullscreen, full, count * sizeof(workarea_t));
}

void workarea_deinit(void) {
  free(workareas);
  free(workareas_fullscreen);
  workarea_count = 0;
}
