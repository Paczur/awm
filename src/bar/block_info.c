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
static block_geometry_t block_info_geometry[MAX_INFO_BLOCKS];
static xcb_connection_t *conn;

//TODO: more granular updates here and in workspaces

static int block_info_run_with_output(char *cmd, char* out, size_t len) {
  FILE *f;
  int pid;
  int status;
  int fd[2];
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
      if(f && !fgets(out, len, f)) {
        out[strcspn(out, "\n")] = 0;
      }
      waitpid(pid, &status, 0);
      if(WIFEXITED(status)) {
        status = WEXITSTATUS(status);
      }
    }
  }
  return status;
}

static const block_settings_t *block_info_get_settings(size_t n)  {
  return (block_info.state[n].status == 1 || block_info.state[n].status == 33) ?
    &block_info.highlighted : &block_info.normal;
}

//TODO: Make callback to realign minimized on update
static void *block_info_update_periodic(void*) {
  bool updated;
  struct timespec ts =
    (struct timespec) { .tv_nsec = 1000000000/CLOCK_STEPS_PER_SECOND };
  for(size_t i=0; i<MAX_INFO_BLOCKS; i++) {
    if(block_info.data[i].cmd == NULL) {
      block_info.state[i].countdown = -1;
    }
  }
  while(true) {
    updated = false;
    for(size_t i=0; i<MAX_INFO_BLOCKS; i++) {
      if(block_info.state[i].update) {
        block_info_run_with_output(block_info.data[i].cmd,
                                   block_info.state[i].text,
                                   INFO_BLOCK_TEXT_LENGTH);
        block_info.state[i].status = 1;
        block_info.state[i].update = false;
        updated = true;
      } else if(block_info.state[i].countdown == 0) {
        block_info.state[i].status =
          block_info_run_with_output(block_info.data[i].cmd,
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
      xcb_flush(conn);
    }
    nanosleep(&ts, &ts);
  }
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
                     uint16_t min_width, block_settings_t *highlighted,
                     block_settings_t *normal, block_info_data_t *data,
                     size_t count, xcb_connection_t *c) {
  block_info.normal = *normal;
  block_info.highlighted = *highlighted;
  block_info.min_width = min_width;
  conn = c;
  memcpy(block_info.data, data, count*sizeof(block_info_data_t));
  block_info.blocks = malloc(bar_container_count*MAX_INFO_BLOCKS*sizeof(block_t));
  block_info.prev_state = calloc(bar_container_count*MAX_INFO_BLOCKS,
                                 sizeof(block_t));
  for(size_t i=0; i<bar_container_count*MAX_INFO_BLOCKS; i++) {
    block_create(block_info.blocks+i, bar_containers.id[i/MAX_INFO_BLOCKS], font);
  }
  pthread_create(&block_info_thread, NULL, block_info_update_periodic, NULL);
}

void block_info_deinit(void) {
  for(size_t i=0; i<bar_container_count*MAX_INFO_BLOCKS; i++) {
    block_destroy(block_info.blocks+i);
  }
  free(block_info.blocks);
  free(block_info.prev_state);
}
