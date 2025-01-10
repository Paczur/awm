#include "malloc.h"

#include <stdint.h>
#include <stdlib.h>

#include "../mocks/mocks.h"

#define BLOCK_COUNT 64

struct block {
  void *ptr;
  size_t size;
};

static struct block blocks[BLOCK_COUNT] = {0};

static void *fake_malloc(size_t size) {
  for(int i = 0; i < BLOCK_COUNT; i++) {
    if(!blocks[i].ptr) {
      blocks[i].ptr = mock_real(malloc)(size);
      blocks[i].size = size;
      return blocks[i].ptr;
    }
  }
  return NULL;
}

static void fake_free(void *ptr) {
  for(int i = 0; i < BLOCK_COUNT; i++) {
    if(blocks[i].ptr == ptr) {
      mock_real(free)(blocks[i].ptr);
      blocks[i].ptr = NULL;
      return;
    }
  }
}

static void *fake_calloc(size_t nmemb, size_t size) {
  for(int i = 0; i < BLOCK_COUNT; i++) {
    if(!blocks[i].ptr) {
      blocks[i].ptr = mock_real(calloc)(nmemb, size);
      blocks[i].size = size * nmemb;
      return blocks[i].ptr;
    }
  }
  return NULL;
}

static void *fake_realloc(void *ptr, size_t size) {
  for(int i = 0; i < BLOCK_COUNT; i++) {
    if(blocks[i].ptr == ptr) {
      blocks[i].ptr = mock_real(realloc)(ptr, size);
      blocks[i].size = size;
      return blocks[i].ptr;
    }
  }
  return NULL;
}

static void *fake_reallocarray(void *ptr, size_t nmemb, size_t size) {
  for(int i = 0; i < BLOCK_COUNT; i++) {
    if(blocks[i].ptr == ptr) {
      blocks[i].ptr = mock_real(reallocarray)(ptr, nmemb, size);
      blocks[i].size = size * nmemb;
      return blocks[i].ptr;
    }
  }
  return NULL;
}

size_t fake_alloc_clear(void) {
  size_t count = 0;
  for(int i = 0; i < BLOCK_COUNT; i++) {
    if(blocks[i].ptr != NULL) {
      mock_real(free)(blocks[i].ptr);
      blocks[i].ptr = NULL;
      count++;
    }
  }
  return count;
}

size_t fake_alloc_count(void) {
  size_t count = 0;
  for(int i = 0; i < BLOCK_COUNT; i++) {
    if(blocks[i].ptr != NULL) count++;
  }
  return count;
}

CTF_MOCK_GROUP(fake_alloc) = {
  CTF_MOCK_BIND(malloc, fake_malloc),
  CTF_MOCK_BIND(free, fake_free),
  CTF_MOCK_BIND(calloc, fake_calloc),
  CTF_MOCK_BIND(realloc, fake_realloc),
  CTF_MOCK_BIND(reallocarray, fake_reallocarray),
};
