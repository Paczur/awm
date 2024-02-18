#ifndef H_LAYOUT_WORKAREA
#define H_LAYOUT_WORKAREA

#include <stdint.h>
#include <stddef.h>

typedef struct workarea_t workarea_t;

extern workarea_t *workareas;
extern size_t workarea_count;

void workarea_init(const workarea_t*, size_t);
void workarea_deinit(void);

#endif
