#include "bar.h"

#include <dirent.h>
#include <iconv.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../const.h"
#include "../syms.h"
#include "../system/system.h"
#include "bar_x.h"

#define WINDOW_NAME_SIZE 1024
#define MAX_PATH_ENTRIES 10240
#define MAX_PATH_ENTRY_SIZE 64

struct clocked_block {
  const char *cmd;
  u32 time;
  u32 flags;
};

struct cmd {
  int pid;
  int fd;
  FILE *f;
};

static u32 monitor_count;
static struct gc gc;
static struct geometry bars[MAX_MONITOR_COUNT];
static struct font_metrics font_metrics;
static u32 visible = 1;

static u32 mode_blocks[MAX_MONITOR_COUNT];
static u32 mode = 2;
static u32 mode_block_offset;
static u32 focused_monitor;

static u8 workspace_occupied[WORKSPACE_COUNT];
static u32 visible_workspaces[WORKSPACE_COUNT];
static u32 workspace_blocks[MAX_MONITOR_COUNT][WORKSPACE_COUNT];
static u32 workspace_blocks_offset;
static u8 workspace_blocks_mapped[MAX_MONITOR_COUNT][WORKSPACE_COUNT];

static u32 minimized_window_blocks[MAX_MONITOR_COUNT][MINIMIZE_QUEUE_SIZE];
static u32 minimized_windows[MINIMIZE_QUEUE_SIZE];
static u32 minimized_window_count;
static u32 minimized_window_blocks_width;
static char minimized_window_names[MINIMIZE_QUEUE_SIZE][WINDOW_NAME_SIZE];
static u32 minimized_window_name_len[MINIMIZE_QUEUE_SIZE];

static struct clocked_block clocked_blocks_data[] = BAR_CLOCKED_BLOCKS;
static u32 clocked_blocks[MAX_MONITOR_COUNT][LENGTH(clocked_blocks_data)];
static struct {
  u16 output[50];
  u32 output_len;
  i32 status;
  u8 mapped;
} clocked_blocks_state[LENGTH(clocked_blocks_data)];
static u32 stop_thread = 0;
static u32 seconds;
static u32 second_multiple = 1;
static pthread_t thread;
static pthread_mutex_t draw_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t update_mutex = PTHREAD_MUTEX_INITIALIZER;
static u32 clocked_blocks_offset;

static u32 launcher_visible = 0;
static struct launcher_control launcher_controls[] = LAUNCHER_CONTROLS;
static char launcher_path_entries[MAX_PATH_ENTRIES][MAX_PATH_ENTRY_SIZE];
static u32 launcher_path_entry_count;

static char launcher_prompt[BAR_LAUNCHER_PROMPT_LENGTH];
static u32 launcher_prompt_length;
static u32 launcher_prompt_offset;
static u32 launcher_prompt_blocks[MAX_MONITOR_COUNT];
static char launcher_hints[BAR_LAUNCHER_HINT_BLOCKS][MAX_PATH_ENTRY_SIZE];
static u32 launcher_hint_lengths[BAR_LAUNCHER_HINT_BLOCKS];
static u32 launcher_hint_selected;
static u32 launcher_hint_count;
static u32 launcher_hint_blocks[MAX_MONITOR_COUNT][BAR_LAUNCHER_HINT_BLOCKS];
static u8 launcher_hint_blocks_mapped[MAX_MONITOR_COUNT]
                                     [BAR_LAUNCHER_HINT_BLOCKS];

static void populate_path(void) {
  char string[512];
  DIR *stream;
  struct dirent *ent;
  struct stat buf = {0};
  u32 len;
  u32 len2;
  u32 swapped;
  char *path = getenv("PATH");
  for(u32 i = 0, j = 0;; i++) {
    if(path[i] == ':' || path[i] == 0) {
      string[j] = 0;
      stream = opendir(string);
      while((ent = readdir(stream)) != NULL) {
        len = strlen(ent->d_name);
        string[j] = '/';
        memcpy(string + j + 1, ent->d_name, len);
        string[j + len + 1] = 0;
        stat(string, &buf);
        if(!S_ISDIR(buf.st_mode) && buf.st_mode & S_IXUSR) {
          for(u32 k = 0; k < launcher_path_entry_count; k++) {
            if(strcmp(launcher_path_entries[k], ent->d_name) == 0) goto skip;
          }
          strlcpy(launcher_path_entries[launcher_path_entry_count++],
                  ent->d_name, MAX_PATH_ENTRY_SIZE);
        skip:
        }
      }
      closedir(stream);
      if(path[i] == 0) break;
      j = 0;
    } else {
      string[j++] = path[i];
    }
  }

  for(u32 i = 0; i < launcher_path_entry_count - 1; i++) {
    swapped = 0;
    for(u32 j = 0; j < launcher_path_entry_count - 1 - i; j++) {
      len = strlen(launcher_path_entries[j]);
      len2 = strlen(launcher_path_entries[j + 1]);
      if(len > len2 ||
         (len == len2 &&
          strcmp(launcher_path_entries[j], launcher_path_entries[j + 1]) > 0)) {
        strcpy(string, launcher_path_entries[j]);
        strcpy(launcher_path_entries[j], launcher_path_entries[j + 1]);
        strcpy(launcher_path_entries[j + 1], string);
        swapped = 1;
      }
    }
    if(!swapped) break;
  }
}

static u8 prefix_matches(char *prefix, char *arr, u32 size) {
  for(u32 i = 0; i < size; i++) {
    if(prefix[i] != arr[i]) return 0;
  }
  return 1;
}

static u8 comp_strings(char *a, char *b) {
  u32 a_len = strlen(a);
  u32 b_len = strlen(b);
  u32 min = MIN(a_len, b_len);
  u32 val;
  for(u32 i = 0; i < min; i++) {
    val = (a[i] < b[i]) - (a[i] > b[i]);
    if(val) return val;
  }
  return (a_len < b_len) - (a_len > b_len);
}

static void calc_prompt_offset(void) {
  launcher_prompt_offset =
    launcher_prompt_length == 0
      ? (u32)font_metrics.width * 6 + BAR_PADDING * 2
      : font_metrics.width * launcher_prompt_length + BAR_PADDING * 2;
}

static void refresh_hints(void) {
  u32 id;
  u32 prev_x;
  u32 w;
  calc_prompt_offset();
  for(u32 i = launcher_hint_count; i < BAR_LAUNCHER_HINT_BLOCKS; i++) {
    for(u32 j = 0; j < monitor_count; j++) {
      if(!launcher_hint_blocks_mapped[j][i]) continue;
      unmap_window(launcher_hint_blocks[j][i]);
      launcher_hint_blocks_mapped[j][i] = 0;
    }
  }
  for(u32 i = 0; i < monitor_count; i++) {
    prev_x =
      bars[i].x + launcher_prompt_offset + BAR_OUTER_MARGIN + BAR_INNER_MARGIN;
    for(u32 j = 0; j < launcher_hint_count; j++) {
      id = launcher_hint_blocks[i][j];
      w = font_metrics.width * strlen(launcher_hints[j]) + BAR_PADDING * 2;
      reconfigure_window(id, prev_x, w);
      if(!launcher_hint_blocks_mapped[i][j]) {
        map_window(id);
        launcher_hint_blocks_mapped[i][j] = 1;
      }
      prev_x += w + BAR_INNER_INSIDE_MARGIN;
    }
  }
  for(u32 i = 0; i < monitor_count; i++) {
    for(u32 j = 0; j < launcher_hint_count; j++) {
      if(launcher_hint_selected == j) {
        change_window_color(launcher_hint_blocks[i][j], BAR_ACTIVE);
        draw_text(launcher_hint_blocks[i][j], gc.active, font_metrics,
                  launcher_hints[j], strlen(launcher_hints[j]));
      } else {
        change_window_color(launcher_hint_blocks[i][j], BAR_INACTIVE);
        draw_text(launcher_hint_blocks[i][j], gc.inactive, font_metrics,
                  launcher_hints[j], strlen(launcher_hints[j]));
      }
    }
  }
}

static void regenerate_hints(void) {
  u32 w = 0;
  u32 max_w = 0;
  u32 len;
  launcher_hint_count = 0;
  for(u32 i = 0; i < launcher_path_entry_count; i++) {
    if(!prefix_matches(launcher_prompt, launcher_path_entries[i],
                       launcher_prompt_length))
      continue;
    len = strlen(launcher_path_entries[i]);
    if(launcher_hint_count < BAR_LAUNCHER_HINT_BLOCKS) {
      launcher_hint_lengths[launcher_hint_count] = len;
      strcpy(launcher_hints[launcher_hint_count++], launcher_path_entries[i]);
    }
  }

  for(u32 i = 0; i < monitor_count; i++)
    if(bars[i].width > max_w) max_w = bars[i].width;
  for(u32 i = 0; i < launcher_hint_count; i++) {
    w += BAR_PADDING * 2 + font_metrics.width * strlen(launcher_hints[i]);
    if(w > max_w) {
      launcher_hint_count = i;
      break;
    }
  }
  refresh_hints();
}

static void refresh_workspace_blocks(void) {
  if(launcher_visible || !visible) return;
  u32 id;
  char text;
  u32 prev_x;
  for(u32 i = 0; i < WORKSPACE_COUNT; i++) {
    for(u32 j = 0; j < monitor_count; j++) {
      if(workspace_blocks_mapped[j][i] && visible_workspaces[j] != i &&
         !workspace_occupied[i]) {
        unmap_window(workspace_blocks[j][i]);
        workspace_blocks_mapped[j][i] = 0;
      }
    }
  }
  for(u32 i = 0; i < monitor_count; i++) {
    prev_x =
      bars[i].x + mode_block_offset + BAR_OUTER_MARGIN + BAR_INNER_MARGIN;
    for(u32 j = 0; j < WORKSPACE_COUNT; j++) {
      id = workspace_blocks[i][j];
      text = (j + 1) % 10 + '0';
      if(visible_workspaces[i] == j) {
        reposition_window(id, prev_x);
        prev_x +=
          BAR_PADDING * 2 + font_metrics.width + BAR_INNER_INSIDE_MARGIN;
        change_window_color(id, BAR_ACTIVE);
        if(!workspace_blocks_mapped[i][j]) {
          map_window(id);
          workspace_blocks_mapped[i][j] = 1;
        }
        draw_text(id, gc.active, font_metrics, &text, 1);
      } else if(workspace_occupied[j]) {
        reposition_window(id, prev_x);
        prev_x +=
          BAR_PADDING * 2 + font_metrics.width + BAR_INNER_INSIDE_MARGIN;
        change_window_color(id, BAR_INACTIVE);
        if(!workspace_blocks_mapped[i][j]) {
          map_window(id);
          workspace_blocks_mapped[i][j] = 1;
        }
        draw_text(id, gc.inactive, font_metrics, &text, 1);
      }
    }
  }
  workspace_blocks_offset = prev_x;
}

static void refresh_minimized_windows(void) {
  u32 x;
  u32 id;
  u32 w;
  u32 text_length;
  if(launcher_visible || !visible) return;
  for(u32 i = 0; i < minimized_window_count; i++)
    text_length += minimized_window_name_len[i];
  minimized_window_blocks_width =
    text_length * font_metrics.width + minimized_window_count * 2 * BAR_PADDING;
  for(u32 i = 0; i < monitor_count; i++) {
    x = bars[i].width / 2 + bars[i].x - minimized_window_blocks_width / 2;
    for(u32 j = 0; j < minimized_window_count; j++) {
      w = minimized_window_name_len[j] * font_metrics.width + BAR_PADDING * 2;
      id = minimized_window_blocks[i][j];
      reconfigure_window(id, x, w);
      map_window(id);

      x += w + BAR_INNER_INSIDE_MARGIN;
    }
    for(u32 j = minimized_window_count; j < MINIMIZE_QUEUE_SIZE; j++)
      unmap_window(minimized_window_blocks[i][j]);
  }
  for(u32 i = 0; i < monitor_count; i++) {
    for(u32 j = 0; j < minimized_window_count; j++)
      draw_text(minimized_window_blocks[i][j], gc.active, font_metrics,
                minimized_window_names[j], minimized_window_name_len[j]);
  }
}

static void unmap_bar(void) {
  for(u32 i = 0; i < monitor_count; i++) {
    unmap_window(mode_blocks[i]);
    for(u32 j = 0; j < WORKSPACE_COUNT; j++) {
      unmap_window(workspace_blocks[i][j]);
      workspace_blocks_mapped[i][j] = 0;
    }
    for(u32 j = 0; j < minimized_window_count; j++)
      unmap_window(minimized_window_blocks[i][j]);
    for(u32 j = 0; j < LENGTH(clocked_blocks_data); j++) {
      unmap_window(clocked_blocks[i][j]);
      clocked_blocks_state[j].mapped = 0;
    }
  }
}

static void map_bar(void) {
  for(u32 i = 0; i < monitor_count; i++) {
    map_window(mode_blocks[i]);
    for(u32 j = 0; j < minimized_window_count; j++)
      map_window(minimized_window_blocks[i][j]);
  }
  refresh_workspace_blocks();
}

static void refresh_prompt_blocks(void) {
  calc_prompt_offset();
  if(launcher_prompt_length == 0) {
    for(u32 i = 0; i < monitor_count; i++) {
      reconfigure_window(launcher_prompt_blocks[i],
                         bars[i].x + BAR_OUTER_MARGIN, launcher_prompt_offset);
      draw_text(launcher_prompt_blocks[i], gc.inactive, font_metrics, "prompt",
                6);
    }
  } else {
    for(u32 i = 0; i < monitor_count; i++) {
      reconfigure_window(launcher_prompt_blocks[i],
                         bars[i].x + BAR_OUTER_MARGIN, launcher_prompt_offset);
      draw_text(launcher_prompt_blocks[i], gc.active, font_metrics,
                launcher_prompt, launcher_prompt_length);
    }
  }
}

static void run_cmd(struct cmd *cmd, const char *shell) {
  int pid = 0;
  int fd[2];
  if(!pipe(fd)) {
    pid = fork();
    if(pid == 0) {
      close(fd[0]);
      dup2(fd[1], STDOUT_FILENO);
      close(fd[1]);
      execl("/bin/sh", "sh", "-c", shell, NULL);
    } else {
      close(fd[1]);
      cmd->f = fdopen(fd[0], "r");
      cmd->pid = pid;
      cmd->fd = fd[0];
    }
  }
}

static u32 read_from_cmd(struct cmd *cmd, char *output, u32 output_size) {
  u32 output_len;
  if(cmd->f && fgets(output, output_size, cmd->f)) {
    output_len = strcspn(output, "\n");
    output[output_len] = 0;
  }
  return output_len;
}

static i32 status_from_cmd(struct cmd *cmd) {
  i32 status = 0;
  waitpid(cmd->pid, &status, 0);
  if(WIFEXITED(status)) status = WEXITSTATUS(status);
  close(cmd->fd);
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
  return out_index;
}

static u32 gcd(u32 a, u32 b) {
  u32 t;
  while(b != 0) {
    t = a % b;
    a = b;
    b = t;
  }
  return a;
}

static void refresh_clocked(i32 redraw) {
  u32 id;
  u32 offset;
  u32 x = BAR_OUTER_MARGIN;
  pthread_mutex_lock(&draw_mutex);
  for(u32 i = 0; i < LENGTH(clocked_blocks_data); i++) {
    if(clocked_blocks_state[i].output_len > 0)
      x += clocked_blocks_state[i].output_len * font_metrics.width +
           BAR_PADDING * 2 + BAR_INNER_MARGIN;
  }
  clocked_blocks_offset = x - BAR_INNER_MARGIN;
  for(i32 i = LENGTH(clocked_blocks_data) - 1; i >= redraw; i--) {
    if(clocked_blocks_state[i].output_len) {
      offset = clocked_blocks_state[i].output_len * font_metrics.width +
               BAR_PADDING * 2;
      for(u32 j = 0; j < monitor_count; j++) {
        id = clocked_blocks[j][i];
        reconfigure_window(id, bars[j].x + bars[j].width - x, offset);
        if(!clocked_blocks_state[i].mapped) map_window(id);
        if(clocked_blocks_state[i].status > 1 ||
           (clocked_blocks_state[i].status == 1 &&
            clocked_blocks_data[i].flags & BAR_FLAGS_ALWAYS_ACTIVE)) {
          change_window_color(id, BAR_URGENT);
          draw_text_utf16(id, gc.urgent, font_metrics,
                          clocked_blocks_state[i].output,
                          clocked_blocks_state[i].output_len);
        } else if(clocked_blocks_state[i].status == 1 ||
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
      clocked_blocks_state[i].mapped = 1;
      x -= offset + BAR_INNER_MARGIN;
    } else if(clocked_blocks_state[i].mapped) {
      for(u32 j = 0; j < monitor_count; j++) unmap_window(clocked_blocks[j][i]);
      clocked_blocks_state[i].mapped = 0;
    }
  }
  pthread_mutex_unlock(&draw_mutex);
}

static void *thread_loop(void *unused) {
  (void)unused;
  i32 redraw;
  struct cmd cmds[LENGTH(clocked_blocks_data)];
  char temp_buff[LENGTH(clocked_blocks_state[0].output)];
  for(u32 i = 0; i < LENGTH(clocked_blocks_data); i++) {
    second_multiple =
      second_multiple * (clocked_blocks_data[i].time /
                         gcd(second_multiple, clocked_blocks_data[i].time));
  }
  seconds = second_multiple;
  while(!stop_thread) {
    redraw = -1;
    pthread_mutex_lock(&update_mutex);
    for(u32 i = 0; i < LENGTH(clocked_blocks_data); i++) {
      if(seconds % clocked_blocks_data[i].time == 0) {
        if(redraw == -1) redraw = i;
        run_cmd(cmds + i, clocked_blocks_data[i].cmd);
      }
    }
    for(u32 i = 0; i < LENGTH(clocked_blocks_data); i++) {
      if(seconds % clocked_blocks_data[i].time == 0) {
        clocked_blocks_state[i].output_len =
          read_from_cmd(cmds + i, temp_buff, LENGTH(temp_buff));
        clocked_blocks_state[i].output_len =
          utf8_to_utf16(clocked_blocks_state[i].output,
                        LENGTH(clocked_blocks_state[i].output), (u8 *)temp_buff,
                        clocked_blocks_state[i].output_len);
        clocked_blocks_state[i].status = status_from_cmd(cmds + i);
      }
    }
    pthread_mutex_unlock(&update_mutex);
    seconds = (seconds + 1) % (second_multiple + 1);
    if(!launcher_visible && visible && redraw != -1) {
      refresh_clocked(redraw);
      send_changes();
    }
    sleep(1);
  }
  return NULL;
}

void update_workspace(u32 *windows, u32 workspace) {
  if(launcher_visible || !visible) return;
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
  u32 min = MIN(MIN(count, WORKSPACE_COUNT), monitor_count);
  memcpy(visible_workspaces, workspaces, min * sizeof(u32));
  refresh_workspace_blocks();
}

void update_focused_monitor(u32 m) {
  focused_monitor = m;
  update_mode(mode);
}

void update_clocked_block(u32 id) {
  char temp_buff[LENGTH(clocked_blocks_state[0].output)];
  struct cmd cmd;
  pthread_mutex_lock(&update_mutex);
  run_cmd(&cmd, clocked_blocks_data[id].cmd);
  clocked_blocks_state[id].output_len =
    read_from_cmd(&cmd, temp_buff, LENGTH(temp_buff));
  clocked_blocks_state[id].output_len = utf8_to_utf16(
    clocked_blocks_state[id].output, LENGTH(clocked_blocks_state[id].output),
    (u8 *)temp_buff, clocked_blocks_state[id].output_len);
  clocked_blocks_state[id].status = status_from_cmd(&cmd);
  pthread_mutex_unlock(&update_mutex);
  refresh_clocked(id);
}

void update_mode(u32 m) {
  const char text = m == NORMAL_MODE ? '+' : 'I';
  mode = m;
  if(launcher_visible || !visible) return;
  for(u32 i = 0; i < focused_monitor; i++) {
    change_window_color(mode_blocks[i], BAR_INACTIVE);
    draw_text(mode_blocks[i], gc.inactive, font_metrics, &text, 1);
  }
  change_window_color(mode_blocks[focused_monitor], BAR_ACTIVE);
  draw_text(mode_blocks[focused_monitor], gc.active, font_metrics, &text, 1);
  for(u32 i = focused_monitor + 1; i < monitor_count; i++) {
    change_window_color(mode_blocks[i], BAR_INACTIVE);
    draw_text(mode_blocks[i], gc.inactive, font_metrics, &text, 1);
  }
}

void update_minimized_window_name(u32 window) {
  for(u32 i = 0; i < minimized_window_count; i++) {
    if(minimized_windows[i] == window) {
      query_window_name(minimized_windows[i], minimized_window_names[i],
                        minimized_window_name_len + i, BAR_WINDOW_NAME_LENGTH);
      break;
    }
  }
  refresh_minimized_windows();
}

void update_minimized_windows(u32 *windows, u32 count) {
  minimized_window_count = MIN(count, MINIMIZE_QUEUE_SIZE);
  memcpy(minimized_windows, windows, minimized_window_count * sizeof(u32));
  for(u32 i = 0; i < minimized_window_count; i++)
    query_window_name(minimized_windows[i], minimized_window_names[i],
                      minimized_window_name_len + i, BAR_WINDOW_NAME_LENGTH);
  refresh_minimized_windows();
}

void show_launcher(void) {
  pthread_mutex_lock(&draw_mutex);
  launcher_visible = 1;
  launcher_prompt_length = 0;
  launcher_hint_selected = 0;
  unmap_bar();
  for(u32 i = 0; i < monitor_count; i++) {
    for(u32 j = 0; j < BAR_LAUNCHER_HINT_BLOCKS; j++)
      launcher_hint_blocks_mapped[i][j] = 0;
    map_window(launcher_prompt_blocks[i]);
  }
  pthread_mutex_unlock(&draw_mutex);
  refresh_prompt_blocks();
  regenerate_hints();
  refresh_prompt_blocks();
  focus_launcher(launcher_prompt_blocks[0]);
}

void hide_launcher(void) {
  pthread_mutex_lock(&draw_mutex);
  for(u32 i = 0; i < monitor_count; i++) {
    unmap_window(launcher_prompt_blocks[i]);
    for(u32 j = 0; j < BAR_LAUNCHER_HINT_BLOCKS; j++)
      unmap_window(launcher_hint_blocks[i][j]);
    for(u32 j = 0; j < BAR_LAUNCHER_HINT_BLOCKS; j++)
      unmap_window(launcher_hint_blocks[i][j]);
  }
  map_bar();
  launcher_visible = 0;
  pthread_mutex_unlock(&draw_mutex);
  unfocus_launcher();
  redraw_bar();
}

void launcher_handle_key(u8 flags, u8 keycode) {
  const u32 *syms;
  const u32 num = keycode_to_keysyms(keycode, &syms);
  if(flags == FLAGS_NONE) {
    for(u32 i = 0; i < num; i++) {
      for(u32 j = 0; j < LENGTH(launcher_controls); j++) {
        if(launcher_controls[j].keysym == syms[i]) {
          launcher_controls[j].f();
          return;
        }
      }
    }
  }
  if(launcher_prompt_length < BAR_LAUNCHER_PROMPT_LENGTH) {
    launcher_prompt_length +=
      keycode_to_utf8(keycode, launcher_prompt + launcher_prompt_length,
                      BAR_LAUNCHER_PROMPT_LENGTH - launcher_prompt_length);
    launcher_hint_selected = 0;
    regenerate_hints();
    refresh_prompt_blocks();
  }
}

u32 launcher_showing(void) { return launcher_visible; }

void launcher_run(void) {
  system_run_bg(launcher_hint_count == 0
                  ? launcher_prompt
                  : launcher_hints[launcher_hint_selected]);
  hide_launcher();
}

void launcher_erase(void) {
  if(launcher_prompt_length == 0) return;
  launcher_prompt_length--;
  regenerate_hints();
  refresh_prompt_blocks();
}

void launcher_hint_left(void) {
  launcher_hint_selected =
    (launcher_hint_selected + launcher_hint_count - 1) % launcher_hint_count;
  refresh_hints();
}

void launcher_hint_right(void) {
  launcher_hint_selected = (launcher_hint_selected + 1) % launcher_hint_count;
  refresh_hints();
}

void redraw_bar(void) {
  if(launcher_visible) return;
  const struct choice {
    u32 preset;
    u32 color;
    char text;
  } choice = mode == NORMAL_MODE
               ? (struct choice){BAR_ACTIVE, gc.active, '+'}
               : (struct choice){BAR_INACTIVE, gc.inactive, 'I'};
  for(u32 i = 0; i < monitor_count; i++)
    draw_text(mode_blocks[i], choice.color, font_metrics, &choice.text, 1);
  refresh_workspace_blocks();
  for(u32 i = 0; i < monitor_count; i++) {
    for(u32 j = 0; j < minimized_window_count; j++) {
      draw_text(minimized_window_blocks[i][j], gc.active, font_metrics,
                minimized_window_names[j], minimized_window_name_len[j]);
    }
  }
  refresh_clocked(0);
}

u32 get_bar_height(void) {
  return font_metrics.ascent + font_metrics.descent + BAR_PADDING * 2 +
         BAR_OUTER_MARGIN;
}

void bar_visibility(u32 val) {
  if(visible == val) return;
  visible = val;
  if(visible) {
    map_bar();
  } else {
    unmap_bar();
  }
}

void init_bar(const struct geometry *geoms, u32 m_count) {
  const u32 font_id = open_font();
  font_metrics = query_font_metrics(font_id);
  struct geometry geom = {
    .x = 0,
    .y = 0,
    .width = BAR_PADDING * 2 + font_metrics.width,
    .height = BAR_PADDING * 2 + font_metrics.ascent + font_metrics.descent,
  };
  mode_block_offset = BAR_PADDING * 2 + font_metrics.width;
  monitor_count = MIN(m_count, MAX_MONITOR_COUNT);
  memcpy(bars, geoms, monitor_count * sizeof(struct geometry));
  for(u32 i = 0; i < monitor_count; i++) {
    geom.y = geoms[i].y + BAR_OUTER_MARGIN;
    geom.x = geoms[i].x + BAR_OUTER_MARGIN;
    mode_blocks[i] = create_window_geom(geom);
    for(u32 j = 0; j < WORKSPACE_COUNT; j++)
      workspace_blocks[i][j] = create_window_geom(geom);
    for(u32 j = 0; j < LENGTH(clocked_blocks_data); j++)
      clocked_blocks[i][j] = create_window_geom(geom);
    for(u32 j = 0; j < MINIMIZE_QUEUE_SIZE; j++)
      minimized_window_blocks[i][j] = create_window_geom(geom);
    launcher_prompt_blocks[i] = create_window_geom(geom);
    for(u32 j = 0; j < BAR_LAUNCHER_HINT_BLOCKS; j++)
      launcher_hint_blocks[i][j] = create_window_geom(geom);
    map_window(mode_blocks[i]);
  }
  gc = create_gc(font_id, workspace_blocks[0][0]);
  close_font(font_id);
  populate_path();
  pthread_create(&thread, NULL, thread_loop, NULL);
}

void deinit_bar(void) {
  void *val;
  stop_thread = 1;
  pthread_join(thread, &val);
}
