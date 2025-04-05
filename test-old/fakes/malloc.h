#ifndef H_TEST_FAKES_MALLOC
#define H_TEST_FAKES_MALLOC

#include <ctf/ctf.h>

CTF_MOCK_GROUP_EXTERN(fake_alloc);

size_t fake_alloc_clear(void);
size_t fake_alloc_count(void);

#endif
