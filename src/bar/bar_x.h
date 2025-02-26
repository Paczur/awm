#ifndef H_AWM_BAR_X
#define H_AWM_BAR_X

#include "../types.h"

struct block_id {
  u32 window_id;
  u32 gc_id;
};

struct font_metrics {
  u32 width;
  u32 ascent;
  u32 descent;
};

struct block_id create_block(u32 font_id);
struct block_id create_block_geom(u32 font_id, struct geometry geom);
u32 open_font(void);
void close_font(u32 id);
void reposition_block(struct block_id id, u32 x);
struct font_metrics get_font_metrics(u32 id);
void draw_text(struct block_id id, struct font_metrics metrics, const char *str,
               u32 str_len);
void change_block_color(struct block_id id, u32 preset);
void map_block(struct block_id id);
void unmap_block(struct block_id id);

#endif
