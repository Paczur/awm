#ifndef H_RADIX
#define H_RADIX

#define RADIX_NODE_ARRAY_SIZE sizeof(char*)

#include <stdbool.h>
#include <stddef.h>
#include "system_config.h"

typedef unsigned char uchar;

typedef struct radix_node_t {
  bool end: 1;
  size_t length: 7;
  struct radix_node_t *adjacent;
  struct radix_node_t *next;
  union value {
    char array[RADIX_NODE_ARRAY_SIZE];
    char *pointer;
  } value;
} radix_node_t;

typedef struct search_node_t {
  const radix_node_t *node;
  uchar position;
  uchar wrong;
} search_node_t;

extern char radix_hints[MAX_LAUNCHER_HINTS][MAX_WORD_LENGTH];

void radix_populate(radix_node_t**);
void radix_gen_hints(const radix_node_t*, char*, size_t);
void radix_10_shortest_sr(const search_node_t*, char*, size_t);
void radix_gen_hints_sr(const search_node_t*, char*, size_t);
search_node_t *radix_search(const radix_node_t*, const char*, size_t);
search_node_t *radix_search_sr(const search_node_t*, const char*, size_t);
void radix_cleanup(radix_node_t*);
void radix_clear(radix_node_t*);
void radix_unmark(radix_node_t*);
void radix_add(radix_node_t**, const char*, size_t);

#endif
