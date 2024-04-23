#include "block_info.h"

#include <sys/wait.h>
#include <unistd.h>

#define CLOCK_STEPS_PER_SECOND 50
#define INFO_BLOCK_TEXT_LENGTH 40

typedef struct block_info_state_t {
  char text[INFO_BLOCK_TEXT_LENGTH];
  bool update;
  int status;
  int countdown;
} block_info_state_t;

typedef struct block_info_t {
  block_t *blocks;
  pthread_rwlock_t *locks;
  block_settings_t normal;
  block_settings_t highlighted;
  block_settings_t urgent;
  size_t min_width;
  block_info_data_t *data;
  block_info_state_t *state;
} block_info_t;

// TODO: FIX BUGGY RENDERING ON SECOND MONITOR

uint32_t block_info_offset_right;
static size_t block_count;
static block_info_t block_info;
static pthread_t block_info_thread;
static bool info_run_flag;
static bool info_run_finished = false;
static void (*callback)(void);
static int (*get_output)(const char *, char *, size_t);
static block_geometry_t *block_info_geometry;
static xcb_connection_t *conn;

static const block_settings_t *block_info_get_settings(size_t n) {
  switch(block_info.state[n].status) {
  case 1:
  case 33:
    return &block_info.highlighted;
  case 2:
    return &block_info.urgent;
  default:
    return &block_info.normal;
  }
}
static void *block_info_update_periodic(void *) {
  int max;
  struct timespec ts =
  (struct timespec){.tv_nsec = (CLOCK_STEPS_PER_SECOND == 1)
                               ? 999999999
                               : 1000000000 / CLOCK_STEPS_PER_SECOND};
  for(size_t i = 0; i < block_count; i++) {
    if(block_info.data[i].cmd == NULL) {
      pthread_rwlock_wrlock(block_info.locks + i);
      block_info.state[i].countdown = -1;
      pthread_rwlock_unlock(block_info.locks + i);
    }
  }
  info_run_flag = true;
  while(info_run_flag) {
    max = -1;
    for(int i = 0; (size_t)i < block_count; i++) {
      pthread_rwlock_rdlock(block_info.locks + i);
      if(block_info.state[i].update) {
        pthread_rwlock_unlock(block_info.locks + i);
        pthread_rwlock_wrlock(block_info.locks + i);
        get_output(block_info.data[i].cmd, block_info.state[i].text,
                   INFO_BLOCK_TEXT_LENGTH);
        block_info.state[i].status = 1;
        block_info.state[i].update = false;
        max = MAX(i, max);
      } else if(block_info.state[i].countdown == 0) {
        pthread_rwlock_unlock(block_info.locks + i);
        pthread_rwlock_wrlock(block_info.locks + i);
        block_info.state[i].status =
        get_output(block_info.data[i].cmd, block_info.state[i].text,
                   INFO_BLOCK_TEXT_LENGTH);
        block_info.state[i].countdown =
        block_info.data[i].timer * CLOCK_STEPS_PER_SECOND;
        max = MAX(i, max);
      } else if(block_info.state[i].countdown > 0) {
        pthread_rwlock_unlock(block_info.locks + i);
        pthread_rwlock_wrlock(block_info.locks + i);
        block_info.state[i].countdown--;
      }
      pthread_rwlock_unlock(block_info.locks + i);
    }
    if(info_run_flag) {
      if(max >= 0) {
        for(int i = 0; i < max + 1; i++) {
          pthread_rwlock_rdlock(block_info.locks + i);
          block_set_text(block_info.blocks + i, block_info.state[i].text);
        }
        block_geometry_update_right(
        block_info.blocks, block_info_geometry, max + 1,
        ((size_t)max + 1 == block_count)
        ? 0
        : block_combined_width(block_info_geometry + max + 1,
                               block_count - max - 1),
        block_info_get_settings, block_info.min_width);
        block_info_offset_right =
        block_combined_width(block_info_geometry, block_count);
        for(int i = 0; i < max + 1; i++) {
          pthread_rwlock_unlock(block_info.locks + i);
        }
        callback();
        xcb_flush(conn);
      }
      nanosleep(&ts, &ts);
    }
  }
  info_run_finished = true;
  return NULL;
}

void block_info_update_highlight(int n, int delay) {
  for(size_t i = 0; i < block_count; i++) {
    if(block_info.data[i].id == n) {
      pthread_rwlock_wrlock(block_info.locks + i);
      block_info.state[i].countdown = delay * CLOCK_STEPS_PER_SECOND;
      block_info.state[i].update = true;
      pthread_rwlock_unlock(block_info.locks + i);
      break;
    }
  }
}

void block_info_update(int n) {
  for(size_t i = 0; i < block_count; i++) {
    pthread_rwlock_rdlock(block_info.locks + i);
    if(block_info.data[i].id == n) {
      pthread_rwlock_unlock(block_info.locks + i);
      pthread_rwlock_wrlock(block_info.locks + i);
      block_info.state[i].countdown = 1;
      pthread_rwlock_unlock(block_info.locks + i);
      break;
    }
    pthread_rwlock_unlock(block_info.locks + i);
  }
}

void block_info_redraw(size_t bar) {
  block_redraw_batch(block_info.blocks, block_count, bar);
}

bool block_info_find_redraw(xcb_window_t window) {
  return block_find_redraw(block_info.blocks, block_count, window);
}

void block_info_init(const PangoFontDescription *font, void (*cb)(void),
                     const bar_block_info_init_t *init, xcb_connection_t *c) {
  block_settings(&block_info.normal, &init->normal);
  block_settings(&block_info.highlighted, &init->highlighted);
  block_settings(&block_info.urgent, &init->urgent);
  block_info.min_width = init->min_width;
  callback = cb;
  conn = c;
  get_output = init->get_output;
  for(size_t i = MAX_INFO_BLOCKS - 1; i > 0; i--) {
    if(init->data->cmd != NULL) {
      block_count = i;
      break;
    }
  }
  block_info.data = malloc(block_count * sizeof(block_info_data_t));
  block_info.state = calloc(block_count, sizeof(block_info_state_t));
  block_info_geometry = malloc(block_count * sizeof(block_geometry_t));
  memcpy(block_info.data, init->data,
         init->data_length * sizeof(block_info_data_t));
  block_info.blocks = malloc(block_count * sizeof(block_t));
  block_info.locks = malloc(block_count * sizeof(pthread_rwlock_t));
  for(size_t i = 0; i < block_count; i++) {
    block_create(block_info.blocks + i, font);
    pthread_rwlock_init(block_info.locks + i, NULL);
  }
  pthread_create(&block_info_thread, NULL, block_info_update_periodic, NULL);
}

void block_info_deinit(void) {
  info_run_flag = false;
  while(!info_run_finished) {
  }
  for(size_t i = 0; i < block_count; i++) {
    block_destroy(block_info.blocks + i);
  }
  free(block_info.blocks);
  free(block_info.data);
  free(block_info.state);
  free(block_info_geometry);
}
