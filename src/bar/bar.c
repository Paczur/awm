#include "bar.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../const.h"
#include "bar_x.h"

struct clocked_block {
  const char *cmd;
  u32 time;
  u32 flags;
};

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

static struct clocked_block clocked_blocks_data[] = BAR_CLOCKED_BLOCKS;
static u32 clocked_blocks[MAX_MONITOR_COUNT][LENGTH(clocked_blocks_data)];
static struct {
  char output[50];
  u32 output_len;
  i32 status;
} clocked_blocks_state[LENGTH(clocked_blocks_data)];
static u32 stop_thread = 0;
static u32 seconds;
static u32 second_multiple = 1;
static pthread_t thread;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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
      }
    }
  }
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

static void *thread_loop(void *unused) {
  (void)unused;
  u32 id;
  u32 x;
  u32 offset;
  i32 redraw;
  for(u32 i = 0; i < LENGTH(clocked_blocks_data); i++)
    second_multiple *= clocked_blocks_data[i].time;
  seconds = second_multiple;
  while(!stop_thread) {
    redraw = -1;
    x = 0;
    pthread_mutex_lock(&mutex);
    for(u32 i = 0; i < LENGTH(clocked_blocks_data); i++) {
      if(seconds % clocked_blocks_data[i].time == 0) {
        if(redraw == -1) redraw = i;
        clocked_blocks_state[i].output_len = 0;
        clocked_blocks_state[i].status = run_with_output(
          clocked_blocks_data[i].cmd, clocked_blocks_state[i].output,
          &clocked_blocks_state[i].output_len,
          sizeof(clocked_blocks_state[i].output));
      }
      x += clocked_blocks_state[i].output_len * font_metrics.width +
           BAR_PADDING * 2;
    }
    if(redraw != -1) {
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
              draw_text(id, gc.active, font_metrics,
                        clocked_blocks_state[i].output,
                        clocked_blocks_state[i].output_len);
            } else {
              change_window_color(id, BAR_INACTIVE);
              draw_text(id, gc.inactive, font_metrics,
                        clocked_blocks_state[i].output,
                        clocked_blocks_state[i].output_len);
            }
          }
          x -= offset;
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

void redraw_bar(void) {
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
  pthread_mutex_lock(&mutex);
  seconds = second_multiple - 1;
  pthread_mutex_unlock(&mutex);
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
    mode_blocks[i] = create_window_geom(geom);
    for(u32 j = 0; j < WORKSPACE_COUNT; j++)
      workspace_blocks[i][j] = create_window_geom(geom);
    for(u32 j = 0; j < LENGTH(clocked_blocks_data); j++)
      clocked_blocks[i][j] = create_window_geom(geom);
    map_window(mode_blocks[i]);
  }
  gc = create_gc(font_id, workspace_blocks[0][0]);
  close_font(font_id);
  pthread_create(&thread, NULL, thread_loop, NULL);
}
