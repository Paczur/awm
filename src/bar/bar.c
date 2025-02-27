#include "bar.h"

#include <stdio.h>
#include <string.h>

#include "../const.h"
#include "bar_x.h"

static u32 m_count;
static struct gc gc;
static struct geometry bars[MAX_MONITOR_COUNT];
static struct font_metrics font_metrics;
static u32 block_width;

static u8 workspace_occupied[WORKSPACE_COUNT];
static u32 visible_workspaces[WORKSPACE_COUNT];
static u32 workspace_blocks[MAX_MONITOR_COUNT][WORKSPACE_COUNT];

static u32 mode_blocks[MAX_MONITOR_COUNT];
static u32 mode = 2;

static void refresh_workspace_blocks(void) {
  u32 id;
  char text;
  u32 prev_x;
  for(u32 i = 0; i < m_count; i++) {
    prev_x = bars[i].x + block_width;
    for(u32 j = 0; j < WORKSPACE_COUNT; j++) {
      id = workspace_blocks[i][j];
      text = (j + 1) % 10 + '0';
      if(visible_workspaces[i] == j) {
        reposition_window(id, prev_x);
        prev_x += BAR_PADDING * 2 + font_metrics.width;
        change_window_color(id, BAR_ACTIVE);
        map_window(id);
        draw_text(id, gc.active, font_metrics, &text, 1);
      } else if(workspace_occupied[j]) {
        reposition_window(id, prev_x);
        prev_x += BAR_PADDING * 2 + font_metrics.width;
        change_window_color(id, BAR_INACTIVE);
        map_window(id);
        draw_text(id, gc.inactive, font_metrics, &text, 1);
      } else {
        unmap_window(id);
      }
    }
  }
}

void update_workspace(u32 *windows, u32 workspace) {
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
  u32 min = MIN(MIN(count, WORKSPACE_COUNT), m_count);
  memcpy(visible_workspaces, workspaces, min * sizeof(u32));
  refresh_workspace_blocks();
}

void update_mode(u32 m) {
  const struct choice {
    u32 preset;
    u32 color;
    char text;
  } choice = m == NORMAL_MODE ? (struct choice){BAR_ACTIVE, gc.active, '+'}
                              : (struct choice){BAR_INACTIVE, gc.inactive, 'I'};
  if(mode == m) return;
  mode = m;
  for(u32 i = 0; i < m_count; i++) {
    change_window_color(mode_blocks[i], choice.preset);
    draw_text(mode_blocks[i], choice.color, font_metrics, &choice.text, 1);
  }
}

u32 get_bar_height(void) {
  return font_metrics.ascent + font_metrics.descent + BAR_PADDING * 2;
}

void init_bar(const struct geometry *geoms, u32 monitor_count) {
  const u32 font_id = open_font();
  font_metrics = query_font_metrics(font_id);
  struct geometry geom = {
    .x = 0,
    .y = 0,
    .width = BAR_PADDING * 2 + font_metrics.width,
    .height = BAR_PADDING * 2 + font_metrics.ascent + font_metrics.descent,
  };
  block_width = BAR_PADDING * 2 + font_metrics.width;
  m_count = MIN(monitor_count, MAX_MONITOR_COUNT);
  memcpy(bars, geoms, m_count * sizeof(struct geometry));
  for(u32 i = 0; i < m_count; i++) {
    geom.y = geoms[i].y;
    geom.x = geoms[i].x;
    for(u32 j = 0; j < WORKSPACE_COUNT; j++)
      workspace_blocks[i][j] = create_window_geom(geom);
    mode_blocks[i] = create_window_geom(geom);
    map_window(mode_blocks[i]);
  }
  gc = create_gc(font_id, workspace_blocks[0][0]);
  close_font(font_id);
}
