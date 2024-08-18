#ifndef H_LAYOUT_WORKAREA
#define H_LAYOUT_WORKAREA

#include <stddef.h>
#include <stdint.h>

typedef struct workarea_t workarea_t;

extern workarea_t *workareas;
extern workarea_t *workareas_fullscreen;
extern size_t workarea_count;

void workarea_init(const workarea_t *, const workarea_t *, size_t);
void workarea_update(const workarea_t *, const workarea_t *, size_t);
void workarea_deinit(void);

#endif
