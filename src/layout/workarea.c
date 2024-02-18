#include "workarea.h"
#include "structs.h"
#include <stdlib.h>
#include <string.h>

workarea_t *workareas;
size_t workarea_count;

void workarea_init(const workarea_t* ws, size_t count) {
  workareas = malloc(count * sizeof(workarea_t));
  workarea_count = count;
  memcpy(workareas, ws, count*sizeof(workarea_t));
}

void workarea_deinit(void) {
  free(workareas);
  workareas = NULL;
  workarea_count = 0;
}
