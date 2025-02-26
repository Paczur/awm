#include "bar.h"

#include <string.h>

#include "../const.h"
#include "bar_x.h"

static u8 workspace_occupied[WORKSPACE_COUNT];
static u32 visible_workspaces[WORKSPACE_COUNT];
static struct block_id workspace_blocks[MAX_MONITOR_COUNT][WORKSPACE_COUNT];
static u32 m_count;
static struct geometry bars[MAX_MONITOR_COUNT];
static struct font_metrics font_metrics;

static void refresh_workspace_blocks(void) {
  struct block_id id;
  char text;
  u32 prev_x = 0;
  for(u32 i = 0; i < m_count; i++) {
    for(u32 j = 0; j < WORKSPACE_COUNT; j++) {
      id = workspace_blocks[i][j];
      text = (j + 1) % 10 + '0';
      if(visible_workspaces[i] == j) {
        map_block(id);
        reposition_block(id, prev_x);
        prev_x += BAR_PADDING * 2 + font_metrics.width;
        change_block_color(id, BAR_ACTIVE);
        draw_text(id, font_metrics, &text, 1);
      } else if(workspace_occupied[j]) {
        map_block(id);
        reposition_block(id, prev_x);
        prev_x += BAR_PADDING * 2 + font_metrics.width;
        change_block_color(id, BAR_INACTIVE);
        draw_text(id, font_metrics, &text, 1);
      } else {
        unmap_block(id);
      }
    }
  }
}

void update_workspace(u32 *windows, u32 workspace) {
  return;
  for(u32 i = 0; i < WINDOWS_PER_WORKSPACE; i++) {
    if(windows[i] != 0) {
      workspace_occupied[workspace] = 1;
      refresh_workspace_blocks();
      return;
    }
  }
  workspace_occupied[workspace] = 0;
  refresh_workspace_blocks();
}

void update_visible_workspaces(u32 *workspaces, u32 count) {
  return;
  u32 min = MIN(count, WORKSPACE_COUNT);
  memcpy(visible_workspaces, workspaces, min * 32);
  refresh_workspace_blocks();
}

void init_bar(const struct geometry *geoms, u32 monitor_count) {
  const u32 font_id = open_font();
  struct geometry geom = {
    0, 0, BAR_PADDING * 2 + font_metrics.width,
    BAR_PADDING * 2 + font_metrics.ascent + font_metrics.descent};
  font_metrics = get_font_metrics(font_id);
  m_count = MIN(monitor_count, MAX_MONITOR_COUNT);
  memcpy(bars, geoms, m_count * sizeof(struct geometry));
  for(u32 i = 0; i < m_count; i++) {
    geom.y = geoms[i].y;
    for(u32 j = 0; j < WORKSPACE_COUNT; j++) {
      workspace_blocks[i][j] = create_block_geom(font_id, geom);
    }
  }
  close_font(font_id);
}
