#include "block_info.h"
#include <unistd.h>
#include <sys/wait.h>

#define CLOCK_STEPS_PER_SECOND 10
#define INFO_BLOCK_TEXT_LENGTH 40

typedef struct block_info_state_t {
  char text[INFO_BLOCK_TEXT_LENGTH];
  bool update;
  int status;
  int countdown;
} block_info_state_t;

typedef struct block_info_t {
  block_t *blocks;
  block_settings_t normal;
  block_settings_t highlighted;
  block_settings_t urgent;
  size_t min_width;
  block_info_data_t data[MAX_INFO_BLOCKS];
  block_info_state_t state[MAX_INFO_BLOCKS];
} block_info_t;

//TODO: DON'T INITIALIZE EVERYTHING
//TODO: FIX BUGGY RENDERING ON SECOND MONITOR

uint16_t block_info_offset_right;
static block_info_t block_info;
static pthread_t block_info_thread;
static bool info_run_flag;
static bool info_run_finished = false;
static void (*callback)(void);
static int (*get_output)(const char*, char*, size_t);
static block_geometry_t block_info_geometry[MAX_INFO_BLOCKS];
static xcb_connection_t *conn;

static const block_settings_t *block_info_get_settings(size_t n)  {
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

static void *block_info_update_periodic(void*) {
  int max;
  struct timespec ts =
    (struct timespec) { .tv_nsec = 1000000000/CLOCK_STEPS_PER_SECOND };
  for(size_t i=0; i<MAX_INFO_BLOCKS; i++) {
    if(block_info.data[i].cmd == NULL) {
      block_info.state[i].countdown = -1;
    }
  }
  info_run_flag = true;
  while(info_run_flag) {
    max = -1;
    for(int i=0; i<MAX_INFO_BLOCKS; i++) {
      if(block_info.state[i].update) {
        get_output(block_info.data[i].cmd,
                   block_info.state[i].text,
                   INFO_BLOCK_TEXT_LENGTH);
        block_info.state[i].status = 1;
        block_info.state[i].update = false;
        max = MAX(i, max);
      } else if(block_info.state[i].countdown == 0) {
        block_info.state[i].status =
          get_output(block_info.data[i].cmd,
                     block_info.state[i].text,
                     INFO_BLOCK_TEXT_LENGTH);
        block_info.state[i].countdown = block_info.data[i].timer*CLOCK_STEPS_PER_SECOND;
        max = MAX(i, max);
      } else if(block_info.state[i].countdown > 0) {
        block_info.state[i].countdown--;
      }
    }
    if(info_run_flag) {
      if(max >= 0) {
        for(int i=0; i<max+1; i++) {
          block_set_text(block_info.blocks+i, block_info.state[i].text);
        }
        block_geometry_update_right(block_info.blocks, block_info_geometry,
                                    max+1,
                                    (max+1 == MAX_INFO_BLOCKS)?0:
                                    block_combined_width(block_info_geometry+max+1,
                                                         MAX_INFO_BLOCKS-max-1),
                                    block_info_get_settings,
                                    block_info.min_width);
        block_info_offset_right =
          block_combined_width(block_info_geometry, MAX_INFO_BLOCKS);
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
  for(size_t i=0; i<MAX_INFO_BLOCKS; i++) {
    if(block_info.data[i].id == n) {
      block_info.state[i].countdown = delay*CLOCK_STEPS_PER_SECOND;
      block_info.state[i].update = true;
      break;
    }
  }
}

void block_info_update(int n) {
  for(size_t i=0; i<MAX_INFO_BLOCKS; i++) {
    if(block_info.data[i].id == n) {
      block_info.state[i].countdown = 1;
      break;
    }
  }
}

void block_info_redraw(size_t bar) {
  block_redraw_batch(block_info.blocks, MAX_INFO_BLOCKS, bar);
}
bool block_info_find_redraw(xcb_window_t window) {
  return block_find_redraw(block_info.blocks, MAX_INFO_BLOCKS, window);
}

void block_info_init(const PangoFontDescription *font,
                     void (*cb)(void),
                     const bar_block_info_init_t *init,
                     xcb_connection_t *c) {
  block_settings(&block_info.normal, &init->normal);
  block_settings(&block_info.highlighted, &init->highlighted);
  block_settings(&block_info.urgent, &init->urgent);
  block_info.min_width = init->min_width;
  callback = cb;
  conn = c;
  get_output = init->get_output;
  memcpy(block_info.data, init->data, init->data_length*sizeof(block_info_data_t));
  block_info.blocks = malloc(MAX_INFO_BLOCKS*sizeof(block_t));
  for(size_t i=0; i<MAX_INFO_BLOCKS; i++) {
    block_create(block_info.blocks+i, font);
  }
  pthread_create(&block_info_thread, NULL, block_info_update_periodic, NULL);
}

void block_info_deinit(void) {
  info_run_flag = false;
  while(!info_run_finished) {}
  for(size_t i=0; i<MAX_INFO_BLOCKS; i++) {
    block_destroy(block_info.blocks+i);
  }
  free(block_info.blocks);
}
