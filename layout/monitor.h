#ifndef H_LAYOUT_MONITOR
#define H_LAYOUT_MONITOR

#include <xcb/randr.h>

typedef struct monitor_t {
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
} monitor_t;

extern monitor_t *monitors;
extern size_t monitor_count;

void monitor_init(void);
void monitor_deinit();

#endif
