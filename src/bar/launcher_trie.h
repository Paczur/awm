#ifndef H_BAR_LAUNCHER_TRIE
#define H_BAR_LAUNCHER_TRIE

#define LAUNCHER_TRIE_NODE_ARRAY_SIZE sizeof(char*)

#include <stdbool.h>
#include <stddef.h>
#include "launcher_hint.h"

#define MAX_WORD_LENGTH 128

typedef unsigned char uchar;

typedef struct launcher_trie_node_t {
  bool end: 1;
  size_t length: 7;
  struct launcher_trie_node_t *adjacent;
  struct launcher_trie_node_t *next;
  union value {
    char array[LAUNCHER_TRIE_NODE_ARRAY_SIZE];
    char *pointer;
  } value;
} launcher_trie_node_t;

typedef struct launcher_trie_search_node_t {
  const launcher_trie_node_t *node;
  uchar position;
  uchar wrong;
} launcher_trie_search_node_t;

extern char launcher_trie_hints[MAX_LAUNCHER_HINTS][MAX_WORD_LENGTH];
extern launcher_trie_node_t* launcher_trie_tree;

void launcher_trie_populate(launcher_trie_node_t**);
void launcher_trie_gen_hints(const launcher_trie_node_t*, char*, size_t);
void launcher_trie_gen_hints_sr(const launcher_trie_search_node_t*, char*, size_t);
launcher_trie_search_node_t *launcher_trie_search(const launcher_trie_node_t*, const char*, size_t);
launcher_trie_search_node_t *launcher_trie_search_sr(const launcher_trie_search_node_t*, const char*, size_t);
void launcher_trie_cleanup(launcher_trie_node_t*);
void launcher_trie_clear(launcher_trie_node_t*);
void launcher_trie_unmark(launcher_trie_node_t*);

#endif
