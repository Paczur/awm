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

CTF_MOCK_VOID_RET_EXTERN(map_window, (u32 window));
CTF_MOCK_VOID_RET_EXTERN(unmap_window, (u32 window));
CTF_MOCK_VOID_RET_EXTERN(configure_window,
                         (u32 window, u32 x, u32 y, u32 width, u32 height));
CTF_MOCK_VOID_RET_EXTERN(query_workspaces, (u32 * windows));
CTF_MOCK_VOID_RET_EXTERN(send_current_workspace, (u32 workspace));
CTF_MOCK_VOID_ARG_EXTERN(u32, query_current_workspace);
CTF_MOCK_VOID_RET_EXTERN(send_workspace, (u32 * windows, u32 w));
CTF_MOCK_GROUP_EXTERN(layout_x_mocks);

#endif
