#ifndef H_AWM_BAR_X
#define H_AWM_BAR_X

#include "../types.h"

struct block_id {
  u32 window_id;
  u32 gc_id;
};

struct font_metrics {
  u16 width;
  u16 ascent;
  u16 descent;
};

struct gc {
  u32 active;
  u32 inactive;
  u32 urgent;
};

u32 create_window(void);
u32 create_window_geom(struct geometry geom);
struct gc create_gc(u32 font_id, u32 window);
u32 open_font(void);
void close_font(u32 id);
void reposition_window(u32 id, u32 x);
struct font_metrics query_font_metrics(u32 id);
void draw_text(u32 window, u32 gc, struct font_metrics metrics, const char *str,
               u32 str_len);
void change_window_color(u32 window, u32 preset);
void map_window(u32 window);
void unmap_window(u32 window);

#endif
