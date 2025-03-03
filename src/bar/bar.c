#include "bar.h"

#include <iconv.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../const.h"
#include "bar_x.h"

#define WINDOW_NAME_MAP_SIZE 1024

struct clocked_block {
  const char *cmd;
  u32 time;
  u32 flags;
};

struct map_entry {
  u32 window;
  u32 name_length;
  char name[BAR_WINDOW_NAME_LENGTH];
};

static u32 m_count;
static struct gc gc;
static struct geometry bars[MAX_MONITOR_COUNT];
static struct font_metrics font_metrics;
static struct map_entry window_name_map[WINDOW_NAME_MAP_SIZE];
static u32 window_name_map_size;

static u32 mode_blocks[MAX_MONITOR_COUNT];
static u32 mode = 2;
static u32 mode_block_offset;
static u32 focused_monitor;

static u8 workspace_occupied[WORKSPACE_COUNT];
static u32 visible_workspaces[WORKSPACE_COUNT];
static u32 workspace_blocks[MAX_MONITOR_COUNT][WORKSPACE_COUNT];
static u32 workspace_blocks_offset;

static u32 minimized_window_blocks[MAX_MONITOR_COUNT][MINIMIZE_QUEUE_SIZE];
static u32 minimized_windows[MINIMIZE_QUEUE_SIZE];
static u32 minimized_window_count;
static u32 minimized_window_blocks_width;

static struct clocked_block clocked_blocks_data[] = BAR_CLOCKED_BLOCKS;
static u32 clocked_blocks[MAX_MONITOR_COUNT][LENGTH(clocked_blocks_data)];
static struct {
  u16 output[50];
  u32 output_len;
  i32 status;
} clocked_blocks_state[LENGTH(clocked_blocks_data)];
static u32 stop_thread = 0;
static u32 seconds;
static u32 second_multiple = 1;
static pthread_t thread;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static u32 clocked_blocks_offset;

static struct map_entry *get_map_entry(u32 window) {
  u32 index = window % WINDOW_NAME_MAP_SIZE;
  if(window_name_map_size > 0) {
    while(window_name_map[index].window != 0) {
      if(window_name_map[index].window == window)
        return window_name_map + index;
      index = (index + 1) % WINDOW_NAME_MAP_SIZE;
    }
  }
  window_name_map[index].window = window;
  query_window_name(window, window_name_map[index].name,
                    &window_name_map[index].name_length,
                    BAR_WINDOW_NAME_LENGTH);
  printf("%s\n", window_name_map[index].name);
  return window_name_map + index;
}

static void refresh_workspace_blocks(void) {
  u32 id;
  char text;
  u32 prev_x;
  for(u32 i = 0; i < m_count; i++) {
    for(u32 j = 0; j < WORKSPACE_COUNT; j++) {
      if(visible_workspaces[i] != j && !workspace_occupied[j])
        unmap_window(workspace_blocks[i][j]);
    }
  }
  for(u32 i = 0; i < m_count; i++) {
    prev_x =
      bars[i].x + mode_block_offset + BAR_OUTER_MARGIN + BAR_INNER_MARGIN;
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
      }
    }
  }
  workspace_blocks_offset = prev_x;
}

static i32 run_with_output(const char *cmd, char *output, u32 *output_len,
                           u32 output_size) {
  FILE *f;
  int pid = 0;
  int fd[2];
  i32 status = 0;
  if(!pipe(fd)) {
    pid = fork();
    if(pid == 0) {
      close(fd[0]);
      dup2(fd[1], STDOUT_FILENO);
      close(fd[1]);
      execl("/bin/sh", "sh", "-c", cmd, NULL);
    } else {
      close(fd[1]);
      f = fdopen(fd[0], "r");
      if(f && fgets(output, output_size, f)) {
        *output_len = strcspn(output, "\n");
        output[*output_len] = 0;
      }
      waitpid(pid, &status, 0);
      if(WIFEXITED(status)) status = WEXITSTATUS(status);
      close(fd[0]);
    }
  }
  return status;
}

static u32 utf8_to_utf16(u16 *out, u32 out_size, u8 *in, u32 in_len) {
  u32 index = 0;
  u32 out_index = 0;
  while(index < in_len && out_index < out_size) {
    if(in[index] & (1 << 7) && in[index] & (1 << 6)) {  // >= 2 bytes
      if(in[index] & (1 << 5)) {                        // >= 3 bytes
        if(in[index] & (1 << 4)) {                      // 4 bytes
          out[out_index++] = '?';
          index += 4;
        } else {  // 3 bytes
          out[out_index++] = ((in[index] & 0x0F) << 12) |
                             ((in[index + 1] & 0x3F) << 6) |
                             (in[index + 2] & 0x3F);
          index += 3;
        }
      } else {  // 2 bytes
        out[out_index++] = ((in[index] & 0x1F) << 6) | (in[index + 1] & 0x3F);
        index += 2;
      }
    } else {  // 1 byte
      out[out_index++] = in[index] & 0x7F;
      index++;
    }
  }
  for(u32 i = 0; i < out_index; i++) {
    out[i] = (out[i] >> 8) | (out[i] << 8);
  }
  fflush(stdout);
  return out_index;
}

static void *thread_loop(void *unused) {
  (void)unused;
  u32 id;
  u32 x;
  u32 offset;
  i32 redraw;
  char temp_buff[LENGTH(clocked_blocks_state[0].output)];
  for(u32 i = 0; i < LENGTH(clocked_blocks_data); i++)
    second_multiple *= clocked_blocks_data[i].time;
  seconds = second_multiple;
  while(!stop_thread) {
    redraw = -1;
    x = BAR_OUTER_MARGIN;
    pthread_mutex_lock(&mutex);
    for(u32 i = 0; i < LENGTH(clocked_blocks_data); i++) {
      if(seconds % clocked_blocks_data[i].time == 0) {
        if(redraw == -1) redraw = i;
        clocked_blocks_state[i].output_len = 0;
        clocked_blocks_state[i].status = run_with_output(
          clocked_blocks_data[i].cmd, temp_buff,
          &clocked_blocks_state[i].output_len, LENGTH(temp_buff));
        clocked_blocks_state[i].output_len =
          utf8_to_utf16(clocked_blocks_state[i].output,
                        LENGTH(clocked_blocks_state[i].output), (u8 *)temp_buff,
                        clocked_blocks_state[i].output_len);
      }
      if(clocked_blocks_state[i].output_len > 0)
        x += clocked_blocks_state[i].output_len * font_metrics.width +
             BAR_PADDING * 2 + BAR_INNER_MARGIN;
    }
    x -= BAR_INNER_MARGIN;
    if(redraw != -1) {
      clocked_blocks_offset = x;
      for(i32 i = LENGTH(clocked_blocks_data) - 1; i >= redraw; i--) {
        if(clocked_blocks_state[i].output_len) {
          offset = clocked_blocks_state[i].output_len * font_metrics.width +
                   BAR_PADDING * 2;
          for(u32 j = 0; j < m_count; j++) {
            id = clocked_blocks[j][i];
            reconfigure_window(id, bars[j].x + bars[j].width - x, offset);
            map_window(id);
            if(clocked_blocks_state[i].status ||
               clocked_blocks_data[i].flags & BAR_FLAGS_ALWAYS_ACTIVE) {
              change_window_color(id, BAR_ACTIVE);
              draw_text_utf16(id, gc.active, font_metrics,
                              clocked_blocks_state[i].output,
                              clocked_blocks_state[i].output_len);
            } else {
              change_window_color(id, BAR_INACTIVE);
              draw_text_utf16(id, gc.inactive, font_metrics,
                              clocked_blocks_state[i].output,
                              clocked_blocks_state[i].output_len);
            }
          }
          x -= offset + BAR_INNER_MARGIN;
        } else {
          for(u32 j = 0; j < m_count; j++) unmap_window(clocked_blocks[j][i]);
        }
      }
    }
    seconds++;
    pthread_mutex_unlock(&mutex);
    send_changes();
    sleep(1);
  }
  return NULL;
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

void update_focused_monitor(u32 m) {
  focused_monitor = m;
  update_mode(mode);
}

void update_mode(u32 m) {
  const char text = m == NORMAL_MODE ? '+' : 'I';
  mode = m;
  for(u32 i = 0; i < focused_monitor; i++) {
    change_window_color(mode_blocks[i], BAR_INACTIVE);
    draw_text(mode_blocks[i], gc.inactive, font_metrics, &text, 1);
  }
  change_window_color(mode_blocks[focused_monitor], BAR_ACTIVE);
  draw_text(mode_blocks[focused_monitor], gc.active, font_metrics, &text, 1);
  for(u32 i = focused_monitor + 1; i < m_count; i++) {
    change_window_color(mode_blocks[i], BAR_INACTIVE);
    draw_text(mode_blocks[i], gc.inactive, font_metrics, &text, 1);
  }
}

void update_minimized_windows(u32 *windows, u32 count) {
  u32 text_length = 0;
  u32 x;
  u32 id;
  u32 w;
  struct map_entry *minimized_window_names[MINIMIZE_QUEUE_SIZE];
  minimized_window_count = MIN(count, MINIMIZE_QUEUE_SIZE);
  memcpy(minimized_windows, windows, minimized_window_count * sizeof(u32));
  for(u32 i = 0; i < minimized_window_count; i++) {
    minimized_window_names[i] = get_map_entry(minimized_windows[i]);
    text_length += minimized_window_names[i]->name_length;
  }
  minimized_window_blocks_width =
    text_length * font_metrics.width + minimized_window_count * 2 * BAR_PADDING;
  for(u32 i = 0; i < m_count; i++) {
    x = bars[i].width / 2 + bars[i].x - minimized_window_blocks_width / 2;
    for(u32 j = 0; j < minimized_window_count; j++) {
      w = minimized_window_names[j]->name_length * font_metrics.width +
          BAR_PADDING * 2;
      id = minimized_window_blocks[i][j];
      reconfigure_window(id, x, w);
      map_window(id);
      draw_text(id, gc.active, font_metrics, minimized_window_names[j]->name,
                minimized_window_names[j]->name_length);
      x += w;
    }
    for(u32 j = minimized_window_count; j < MINIMIZE_QUEUE_SIZE; j++)
      unmap_window(minimized_window_blocks[i][j]);
  }
}

void redraw_bar(void) {
  struct map_entry *minimized_window_names[MINIMIZE_QUEUE_SIZE];
  for(u32 i = 0; i < minimized_window_count; i++)
    minimized_window_names[i] = get_map_entry(minimized_windows[i]);
  const struct choice {
    u32 preset;
    u32 color;
    char text;
  } choice = mode == NORMAL_MODE
               ? (struct choice){BAR_ACTIVE, gc.active, '+'}
               : (struct choice){BAR_INACTIVE, gc.inactive, 'I'};
  for(u32 i = 0; i < m_count; i++)
    draw_text(mode_blocks[i], choice.color, font_metrics, &choice.text, 1);
  refresh_workspace_blocks();
  for(u32 i = 0; i < m_count; i++) {
    for(u32 j = 0; j < minimized_window_count; j++) {
      draw_text(minimized_window_blocks[i][j], gc.active, font_metrics,
                minimized_window_names[j]->name,
                minimized_window_names[j]->name_length);
    }
  }
  pthread_mutex_lock(&mutex);
  seconds = second_multiple;
  pthread_mutex_unlock(&mutex);
}

u32 get_bar_height(void) {
  return font_metrics.ascent + font_metrics.descent + BAR_PADDING * 2 +
         BAR_OUTER_MARGIN;
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
  mode_block_offset = BAR_PADDING * 2 + font_metrics.width;
  m_count = MIN(monitor_count, MAX_MONITOR_COUNT);
  memcpy(bars, geoms, m_count * sizeof(struct geometry));
  for(u32 i = 0; i < m_count; i++) {
    geom.y = geoms[i].y + BAR_OUTER_MARGIN;
    geom.x = geoms[i].x + BAR_OUTER_MARGIN;
    mode_blocks[i] = create_window_geom(geom);
    for(u32 j = 0; j < WORKSPACE_COUNT; j++)
      workspace_blocks[i][j] = create_window_geom(geom);
    for(u32 j = 0; j < LENGTH(clocked_blocks_data); j++)
      clocked_blocks[i][j] = create_window_geom(geom);
    for(u32 j = 0; j < MINIMIZE_QUEUE_SIZE; j++)
      minimized_window_blocks[i][j] = create_window_geom(geom);
    map_window(mode_blocks[i]);
  }
  gc = create_gc(font_id, workspace_blocks[0][0]);
  close_font(font_id);
  pthread_create(&thread, NULL, thread_loop, NULL);
}

void deinit_bar(void) {
  void *val;
  stop_thread = 1;
  pthread_join(thread, &val);
}
