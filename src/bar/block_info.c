#include "block_info.h"
#include "bar_container.h"
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
  size_t min_width;
  block_info_data_t data[MAX_INFO_BLOCKS];
  block_info_state_t state[MAX_INFO_BLOCKS];
  bool *prev_state;
} block_info_t;

uint16_t block_info_offset_right;
static block_info_t block_info;
static pthread_t block_info_thread;
static bool info_run_flag;
static bool info_run_finished = false;
//TODO: Make this callback not block the timer
static void (*callback)(void);
static int (*get_output)(const char*, char*, size_t);
static block_geometry_t block_info_geometry[MAX_INFO_BLOCKS];
//TODO: more granular updates here and in workspaces

static const block_settings_t *block_info_get_settings(size_t n)  {
  return (block_info.state[n].status == 1 || block_info.state[n].status == 33) ?
    &block_info.highlighted : &block_info.normal;
}

static void *block_info_update_periodic(void*) {
  bool updated;
  struct timespec ts =
    (struct timespec) { .tv_nsec = 1000000000/CLOCK_STEPS_PER_SECOND };
  for(size_t i=0; i<MAX_INFO_BLOCKS; i++) {
    if(block_info.data[i].cmd == NULL) {
      block_info.state[i].countdown = -1;
    }
  }
  info_run_flag = true;
  while(info_run_flag) {
    updated = false;
    for(size_t i=0; i<MAX_INFO_BLOCKS; i++) {
      if(block_info.state[i].update) {
        get_output(block_info.data[i].cmd,
                   block_info.state[i].text,
                   INFO_BLOCK_TEXT_LENGTH);
        block_info.state[i].status = 1;
        block_info.state[i].update = false;
        updated = true;
      } else if(block_info.state[i].countdown == 0) {
        block_info.state[i].status =
          get_output(block_info.data[i].cmd,
                     block_info.state[i].text,
                     INFO_BLOCK_TEXT_LENGTH);
        block_info.state[i].countdown = block_info.data[i].timer*CLOCK_STEPS_PER_SECOND;
        updated = true;
      } else if(block_info.state[i].countdown > 0) {
        block_info.state[i].countdown--;
      }
    }
    if(updated) {
      for(size_t i=0; i<bar_container_count*MAX_INFO_BLOCKS; i++) {
        block_set_text(block_info.blocks+i,
                       block_info.state[i/bar_container_count].text);
      }
      block_geometry_update_rightef(block_info.blocks, block_info_geometry,
                                    block_info.prev_state, MAX_INFO_BLOCKS,
                                    block_info_get_settings,
                                    block_info.min_width);
      block_info_offset_right =
        block_combined_width(block_info_geometry, MAX_INFO_BLOCKS);
      callback();
    }
    nanosleep(&ts, &ts);
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

void block_info_redraw(void) {
  block_redraw_batch(block_info.blocks, bar_container_count*MAX_INFO_BLOCKS);
}

void block_info_init(const PangoFontDescription *font,
                     const bar_block_info_init_t *init) {
  block_settings(&block_info.normal, &init->normal);
  block_settings(&block_info.highlighted, &init->highlighted);
  block_info.min_width = init->min_width;
  callback = init->callback;
  get_output = init->get_output;
  memcpy(block_info.data, init->data, init->data_length*sizeof(block_info_data_t));
  block_info.blocks = malloc(bar_container_count*MAX_INFO_BLOCKS*sizeof(block_t));
  block_info.prev_state = calloc(bar_container_count*MAX_INFO_BLOCKS,
                                 sizeof(block_t));
  for(size_t i=0; i<bar_container_count*MAX_INFO_BLOCKS; i++) {
    block_create(block_info.blocks+i, bar_containers.id[i/MAX_INFO_BLOCKS], font);
  }
  pthread_create(&block_info_thread, NULL, block_info_update_periodic, NULL);
}

void block_info_deinit(void) {
  info_run_flag = false;
  while(!info_run_finished) {}
  for(size_t i=0; i<bar_container_count*MAX_INFO_BLOCKS; i++) {
    block_destroy(block_info.blocks+i);
  }
  free(block_info.blocks);
  free(block_info.prev_state);
}
