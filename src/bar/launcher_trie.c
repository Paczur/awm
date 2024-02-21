#include "launcher_trie.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

static uchar lengths[MAX_LAUNCHER_HINTS];
char launcher_trie_hints[MAX_LAUNCHER_HINTS][MAX_WORD_LENGTH];
launcher_trie_node_t *launcher_trie_tree;

void launcher_trie_add(launcher_trie_node_t **tree, const char *word, size_t length) {
  launcher_trie_node_t *curr;
  launcher_trie_node_t *t;
  char *temp;
  size_t j;
  size_t split;
  if(*tree == NULL ||
     ((*tree)->length <= LAUNCHER_TRIE_NODE_ARRAY_SIZE && (*tree)->value.array[0] > word[0]) ||
     ((*tree)->length > LAUNCHER_TRIE_NODE_ARRAY_SIZE && (*tree)->value.pointer[0] > word[0])) {
    t = *tree;
    *tree = malloc(sizeof(launcher_trie_node_t));
    if(length <= LAUNCHER_TRIE_NODE_ARRAY_SIZE) {
      memcpy((*tree)->value.array, word, length*sizeof(char));
    } else {
      (*tree)->value.pointer = malloc(length * sizeof(char));
      memcpy((*tree)->value.pointer, word, length*sizeof(char));
    }
    (*tree)->length = length;
    (*tree)->end = true;
    (*tree)->next = NULL;
    (*tree)->adjacent = t;
    return;
  }
  curr = *tree;
  for(size_t i=0;;) {
    while((curr->length <= LAUNCHER_TRIE_NODE_ARRAY_SIZE &&
           curr->value.array[0] < word[i]) ||
          (curr->length > LAUNCHER_TRIE_NODE_ARRAY_SIZE &&
           curr->value.pointer[0] < word[i])) {
      if(curr->adjacent == NULL ||
         (curr->adjacent->length <= LAUNCHER_TRIE_NODE_ARRAY_SIZE &&
          curr->adjacent->value.array[0] > word[i]) ||
         (curr->adjacent->length > LAUNCHER_TRIE_NODE_ARRAY_SIZE &&
          curr->adjacent->value.pointer[0] > word[i])) {
        t = curr->adjacent;
        curr->adjacent = malloc(sizeof(launcher_trie_node_t));
        if(length-i <= LAUNCHER_TRIE_NODE_ARRAY_SIZE) {
          memcpy(curr->adjacent->value.array, word+i, (length-i)*sizeof(char));
        } else {
          curr->adjacent->value.pointer = malloc((length-i) * sizeof(char));
          memcpy(curr->adjacent->value.pointer, word+i, (length-i)*sizeof(char));
        }
        curr->adjacent->next = NULL;
        curr->adjacent->length = length-i;
        curr->adjacent->end = true;
        curr->adjacent->adjacent = t;
        return;
      }
      curr = curr->adjacent;
    }
    i++;
    j=1;
    split = false;
    while(j != curr->length &&
          i != length &&
          ((curr->length <= LAUNCHER_TRIE_NODE_ARRAY_SIZE &&
            curr->value.array[j] == word[i]) ||
           (curr->length > LAUNCHER_TRIE_NODE_ARRAY_SIZE &&
            curr->value.pointer[j] == word[i]))) {
      j++; i++;
    }
    if(j != curr->length) {
      t = curr->next;
      curr->next = malloc(sizeof(launcher_trie_node_t));
      if(curr->length-j <= LAUNCHER_TRIE_NODE_ARRAY_SIZE) {
        memcpy(curr->next->value.array,
               (curr->length <= LAUNCHER_TRIE_NODE_ARRAY_SIZE)? curr->value.array+j:
               curr->value.pointer+j,
               (curr->length-j)*sizeof(char));
      } else {
        curr->next->value.pointer = malloc((curr->length-j)*sizeof(char));
        memcpy(curr->next->value.pointer, curr->value.pointer+j,
               (curr->length-j)*sizeof(char));
      }
      curr->next->length = curr->length-j;
      if(j <= LAUNCHER_TRIE_NODE_ARRAY_SIZE) {
        if(curr->length > LAUNCHER_TRIE_NODE_ARRAY_SIZE) {
          temp = curr->value.pointer;
          memcpy(curr->value.array, curr->value.pointer, j*sizeof(char));
          free(temp);
        }
      } else {
        curr->value.pointer = realloc(curr->value.pointer, j*sizeof(char));
      }
      curr->length = j;
      curr->next->next = t;
      curr->next->end = curr->end;
      curr->next->adjacent = NULL;
      curr->end = false;
      split = true;
    }
    if(i == length) {
      curr->end = true;
      break;
    } else if(split) {
      curr->length = j;
      if((curr->next->length <= LAUNCHER_TRIE_NODE_ARRAY_SIZE &&
          curr->next->value.array[0] > word[i]) ||
         (curr->next->length > LAUNCHER_TRIE_NODE_ARRAY_SIZE &&
          curr->next->value.pointer[0] > word[i])) {
        t = curr->next;
        curr->next = malloc(sizeof(launcher_trie_node_t));
        if(length-i <= LAUNCHER_TRIE_NODE_ARRAY_SIZE) {
          memcpy(curr->next->value.array, word+i, (length-i)*sizeof(char));
        } else {
          curr->next->value.pointer = malloc((length-i)*sizeof(char));
          memcpy(curr->next->value.pointer, word+i, (length-i)*sizeof(char));
        }
        curr->next->end = true;
        curr->next->length = length-i;
        curr->next->next = NULL;
        curr->next->adjacent = t;
      } else {
        curr = curr->next;
        while(curr->adjacent != NULL &&
              ((curr->adjacent->length <= LAUNCHER_TRIE_NODE_ARRAY_SIZE &&
                curr->adjacent->value.array[0] < word[i]) ||
               (curr->adjacent->length > LAUNCHER_TRIE_NODE_ARRAY_SIZE &&
                curr->adjacent->value.pointer[0] < word[i]))) {
          curr = curr->adjacent;
        }
        t = curr->adjacent;
        curr->adjacent = malloc(sizeof(launcher_trie_node_t));
        if(length-i <= LAUNCHER_TRIE_NODE_ARRAY_SIZE) {
          memcpy(curr->adjacent->value.array, word+i, (length-i)*sizeof(char));
        } else {
          curr->adjacent->value.pointer = malloc((length-i)*sizeof(char));
          memcpy(curr->adjacent->value.pointer, word+i, (length-i)*sizeof(char));
        }
        curr->adjacent->length = length-i;
        curr->adjacent->end = true;
        curr->adjacent->next = NULL;
        curr->adjacent->adjacent = t;
      }
      break;
    }
    if(curr->next == NULL ||
       (curr->next->length <= LAUNCHER_TRIE_NODE_ARRAY_SIZE &&
        curr->next->value.array[0] > word[i]) ||
       (curr->next->length > LAUNCHER_TRIE_NODE_ARRAY_SIZE &&
        curr->next->value.pointer[0] > word[i])) {
      t = curr->next;
      curr->next = malloc(sizeof(launcher_trie_node_t));
      if(length-i <= LAUNCHER_TRIE_NODE_ARRAY_SIZE) {
        memcpy(curr->next->value.array, word+i, length-i);
      } else {
        curr->next->value.pointer = malloc((length-i) * sizeof(char));
        memcpy(curr->next->value.pointer, word+i, length-i);
      }
      curr->next->length = length-i;
      curr->next->next = NULL;
      curr->next->end = true;
      curr->next->adjacent = t;
      break;
    }
    curr = curr->next;
  }
}

void launcher_trie_unmark(launcher_trie_node_t *node) {
  node->end = false;
  launcher_trie_node_t *curr = node;
  while(curr != NULL) {
    if(curr->next != NULL)
      launcher_trie_unmark(curr->next);
    curr = curr->adjacent;
  }
}

void launcher_trie_deep_cleanup(launcher_trie_node_t *node) {
  launcher_trie_node_t *curr = node;
  while(curr != NULL) {
    if(curr->next != NULL) {
      launcher_trie_deep_cleanup(curr->next);
      while(curr->next != NULL && curr->next->next == NULL && !curr->next->end) {
        launcher_trie_node_t *t = curr->next;
        curr->next = t->adjacent;
        if(t->length > LAUNCHER_TRIE_NODE_ARRAY_SIZE) free(t->value.pointer);
        free(t);
      }
    }
    curr = curr->adjacent;
  }
}

void launcher_trie_cleanup(launcher_trie_node_t *node) {
  launcher_trie_node_t *curr = node;
  launcher_trie_deep_cleanup(node);
  while(curr->next != NULL && curr->next->next == NULL && !curr->next->end) {
    launcher_trie_node_t *t = curr->next;
    curr->next = t->adjacent;
    if(t->length > LAUNCHER_TRIE_NODE_ARRAY_SIZE) free(t->value.pointer);
    free(t);
  }
}

void launcher_trie_clear(launcher_trie_node_t *node) {
  launcher_trie_node_t* curr = node;
  launcher_trie_node_t* t;
  while(curr != NULL) {
    if(curr->next != NULL) launcher_trie_clear(curr->next);
    t = curr;
    curr = curr->adjacent;
    if(t->length > LAUNCHER_TRIE_NODE_ARRAY_SIZE)
      free(t->value.pointer);
    free(t);
  }
}

void launcher_trie_print(const launcher_trie_node_t *node) {
  if(node == NULL) {
    puts("NULL");
    return;
  }
  printf("%20.*s %d ", (int)node->length,
         (node->length<=LAUNCHER_TRIE_NODE_ARRAY_SIZE)?node->value.array:
         node->value.pointer, node->end);
  if(node->adjacent == NULL) {
    printf("%20s", "NULL");
  } else {
    printf("%20.*s", (int)node->adjacent->length,
           (node->adjacent->length<=LAUNCHER_TRIE_NODE_ARRAY_SIZE)?node->adjacent->value.array:
           node->adjacent->value.pointer);
  }
  printf(" ");
  if(node->next == NULL) {
    printf("%20s", "NULL");
  } else {
    printf("%20.*s", (int)node->next->length,
           (node->next->length<=LAUNCHER_TRIE_NODE_ARRAY_SIZE)?node->next->value.array:
           node->next->value.pointer);
  }
  puts("");
  if(node->adjacent != NULL) {
    launcher_trie_print(node->adjacent);
  }
  if(node->next != NULL) {
    puts("");
    launcher_trie_print(node->next);
  }
}

launcher_trie_search_node_t *launcher_trie_search(const launcher_trie_node_t *tree,
                            const char *word, size_t length) {
  launcher_trie_search_node_t *search;
  const launcher_trie_node_t *curr = tree;
  size_t j;
  for(size_t i=0;;) {
    while(curr != NULL &&
          ((curr->length <= LAUNCHER_TRIE_NODE_ARRAY_SIZE &&
            curr->value.array[0] != word[i]) ||
           (curr->length > LAUNCHER_TRIE_NODE_ARRAY_SIZE &&
            curr->value.pointer[0] != word[i])))
      curr = curr->adjacent;
    if(curr == NULL)
      return NULL;
    i++;
    j = 1;
    while(j != curr->length && i != length &&
          ((curr->length <= LAUNCHER_TRIE_NODE_ARRAY_SIZE &&
            curr->value.array[j] == word[i]) ||
           (curr->length > LAUNCHER_TRIE_NODE_ARRAY_SIZE &&
            curr->value.pointer[j] == word[i]))){
      j++; i++;
    }
    if(j == curr->length && i != length) {
      curr = curr->next;
    } else {
      search = malloc(sizeof(launcher_trie_search_node_t));
      search->node = curr;
      search->position = j;
      search->wrong = length-i;
      return search;
    }
  }
}

launcher_trie_search_node_t
*launcher_trie_search_sr(const launcher_trie_search_node_t *node,
                         const char *word, size_t length) {
  const launcher_trie_node_t *curr = node->node;
  launcher_trie_search_node_t *search;
  size_t j;
  for(size_t i=0;;) {
    j = node->position;
    while(j != curr->length && i != length &&
          ((curr->length <= LAUNCHER_TRIE_NODE_ARRAY_SIZE &&
            curr->value.array[j] == word[i]) ||
           (curr->length > LAUNCHER_TRIE_NODE_ARRAY_SIZE &&
            curr->value.pointer[j] == word[i]))){
      j++; i++;
    }
    if(j == curr->length && i != length) {
      return launcher_trie_search(curr->next, word+i, length-i);
    } else {
      search = malloc(sizeof(launcher_trie_search_node_t));
      search->node = curr;
      search->position = j;
      search->wrong = length-i;
      return search;
    }
  }
}

void launcher_trie_new_shortest(const char *buff, char length) {
  for(size_t i=0; i<LENGTH(lengths); i++) {
    if(lengths[i] > length) {
      memmove(lengths+i+1, lengths+i, (LENGTH(lengths)-(i+1))*sizeof(lengths[0]));
      memmove(launcher_trie_hints+i+1, launcher_trie_hints+i, (LENGTH(launcher_trie_hints)-(i+1))*sizeof(launcher_trie_hints[0]));
      memcpy(launcher_trie_hints+i, buff, length*sizeof(buff[0]));
      lengths[i] = length;
      break;
    }
  }
}

void launcher_trie_10_shortest(const launcher_trie_node_t *node, char* buff,
                               size_t length) {
  const launcher_trie_node_t *curr = node;
  while(curr != NULL) {
    if(curr->next != NULL || curr->end) {
      if(curr->length <= LAUNCHER_TRIE_NODE_ARRAY_SIZE) {
        memcpy(buff+length, curr->value.array, curr->length*sizeof(char));
      } else {
        memcpy(buff+length, curr->value.pointer, curr->length*sizeof(char));
      }
      if(curr->end) {
        launcher_trie_new_shortest(buff, length+curr->length);
      }
      if(curr->next != NULL) {
        launcher_trie_10_shortest(curr->next, buff, length+curr->length);
      }
    }
    curr = curr->adjacent;
  }
}

void launcher_trie_10_shortest_sr(const launcher_trie_search_node_t *node, char *buff, size_t length) {
  if(node == NULL) return;
  const launcher_trie_node_t *curr = node->node;
  length -= node->wrong;
  if(node->position == curr->length) {
    if(curr->end) {
      launcher_trie_new_shortest(buff, length+curr->length-node->position);
    }
    curr = curr->next;
  } else if(node->position > 0) {
    if(curr->length <= LAUNCHER_TRIE_NODE_ARRAY_SIZE) {
      memcpy(buff+length, curr->value.array+node->position,
             curr->length-node->position);
    } else {
      memcpy(buff+length, curr->value.pointer+node->position,
             curr->length-node->position);
    }
    if(curr->end) {
      launcher_trie_new_shortest(buff, length+curr->length-node->position);
    }
    length += curr->length-node->position;
    curr = curr->next;
  }
  while(curr != NULL) {
    if(curr->next != NULL || curr->end) {
      if(curr->length <= LAUNCHER_TRIE_NODE_ARRAY_SIZE) {
        memcpy(buff+length, curr->value.array, curr->length*sizeof(char));
      } else {
        memcpy(buff+length, curr->value.pointer, curr->length*sizeof(char));
      }
      if(curr->end) {
        launcher_trie_new_shortest(buff, length+curr->length);
      }
      if(curr->next != NULL) {
        launcher_trie_10_shortest(curr->next, buff, length+curr->length);
      }
    }
    curr = curr->adjacent;
  }
}

void launcher_trie_gen_hints_sr(const launcher_trie_search_node_t *node, char* buff, size_t length) {
  memset(lengths, UCHAR_MAX, sizeof(lengths));
  launcher_trie_10_shortest_sr(node, buff, length);
  for(size_t i=0; i<MAX_LAUNCHER_HINTS; i++) {
    if(lengths[i] == UCHAR_MAX) {
      lengths[i] = 0;
    }
    launcher_trie_hints[i][lengths[i]] = 0;
  }
}

void launcher_trie_gen_hints(const launcher_trie_node_t *node, char* buff, size_t length) {
  memset(lengths, UCHAR_MAX, sizeof(lengths));
  launcher_trie_10_shortest(node, buff, length);
  for(size_t i=0; i<MAX_LAUNCHER_HINTS; i++) {
    if(lengths[i] == UCHAR_MAX) {
      lengths[i] = 0;
    }
    launcher_trie_hints[i][lengths[i]] = 0;
  }
}

void launcher_trie_populate(launcher_trie_node_t **tree) {
  char string[MAX_WORD_LENGTH];
  DIR *stream;
  struct dirent *ent;
  struct stat buf = {0};
  size_t len;
  char *path = getenv("PATH");
  for(size_t i=0, j=0;; i++) {
    if(path[i] == ':' || path[i] == 0) {
      string[j] = 0;
      stream = opendir(string);
      while((ent = readdir(stream)) != NULL) {
        len = strlen(ent->d_name);
        string[j] = '/';
        memcpy(string+j+1, ent->d_name, len);
        string[j+len+1] = 0;
        stat(string, &buf);
        if(!S_ISDIR(buf.st_mode) && buf.st_mode & S_IXUSR) {
          launcher_trie_add(tree, ent->d_name, len);
        }
      }
      closedir(stream);
      if(path[i] == 0) break;
      j = 0;
    } else {
      string[j++] = path[i];
    }
  }
}
