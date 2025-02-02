#ifndef H_TEST_MOCKS
#define H_TEST_MOCKS

#include <ctf/ctf.h>
#include <types.h>

CTF_MOCK_VOID_EXTERN(grab_keyboard);
CTF_MOCK_VOID_EXTERN(ungrab_keyboard);
CTF_MOCK_VOID_RET_EXTERN(grab_key, (u8 key, u8 mod));
CTF_MOCK_VOID_RET_EXTERN(send_mode, (u8 mode));
CTF_MOCK_VOID_ARG_EXTERN(u8, query_mode);

CTF_MOCK_GROUP_EXTERN(shortcut_x_mocks);

CTF_MOCK_VOID_RET_EXTERN(set_mode, (u8 mode));
#endif
