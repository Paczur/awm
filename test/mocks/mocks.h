#ifndef H_TEST_MOCKS
#define H_TEST_MOCKS

#include <ctf/ctf.h>
#include <types.h>

CTF_MOCK_VOID_EXTERN(grab_keyboard);
CTF_MOCK_VOID_EXTERN(ungrab_keyboard);
CTF_MOCK_VOID_RET_EXTERN(grab_key, (u8 key, u8 mod));
CTF_MOCK_VOID_RET_EXTERN(send_mode, (u8 mode));
CTF_MOCK_VOID_ARG_EXTERN(u8, query_mode);
CTF_MOCK_VOID_ARG_EXTERN(u32, insert_mode_allowed);
CTF_MOCK_EXTERN(u32, keycode_to_keysyms, (u8 keycode, const u32 **syms));
CTF_MOCK_VOID_ARG_EXTERN(u8, keymap_min_keycode);
CTF_MOCK_VOID_ARG_EXTERN(u8, keymap_max_keycode);
CTF_MOCK_GROUP_EXTERN(shortcut_x_mocks);

CTF_MOCK_VOID_RET_EXTERN(map_window, (u32 window));
CTF_MOCK_VOID_RET_EXTERN(unmap_window, (u32 window));
CTF_MOCK_VOID_RET_EXTERN(configure_window,
                         (u32 window, u32 x, u32 y, u32 width, u32 height));
CTF_MOCK_VOID_RET_EXTERN(query_workspaces, (u32 * windows));
CTF_MOCK_VOID_RET_EXTERN(send_visible_workspaces,
                         (u32 * workspaces, u32 count));
CTF_MOCK_VOID_RET_EXTERN(query_visible_workspaces,
                         (u32 * workspaces, u32 count));
CTF_MOCK_VOID_RET_EXTERN(send_workspace, (u32 * windows, u32 w));
CTF_MOCK_GROUP_EXTERN(layout_x_mocks);
CTF_MOCK_VOID_RET_EXTERN(change_window_border_color, (u32 window, u32 color));
CTF_MOCK_VOID_RET_EXTERN(listen_to_events, (u32 window));
CTF_MOCK_VOID_RET_EXTERN(focus_window, (u32 window));
CTF_MOCK_VOID_EXTERN(unfocus_window);
CTF_MOCK_VOID_RET_EXTERN(send_focused_window, (u32 window));
CTF_MOCK_VOID_RET_EXTERN(send_focused_monitor, (u32 window));
CTF_MOCK_VOID_ARG_EXTERN(u32, query_focused_monitor);
CTF_MOCK_VOID_RET_EXTERN(send_focused_workspace, (u32 w));
CTF_MOCK_VOID_RET_EXTERN(send_focused_windows, (u32 * windows));
CTF_MOCK_VOID_RET_EXTERN(query_focused_windows, (u32 * windows));

CTF_MOCK_VOID_RET_EXTERN(send_minimized_windows, (u32 * windows, u32 len));
CTF_MOCK_VOID_RET_EXTERN(query_minimized_windows, (u32 * windows, u32 len));
CTF_MOCK_VOID_ARG_EXTERN(u32, query_minimized_window_count);
CTF_MOCK_VOID_RET_EXTERN(query_size_offsets, (i32 * offsets));
CTF_MOCK_VOID_RET_EXTERN(send_size_offsets, (i32 * offsets));

CTF_MOCK_VOID_RET_EXTERN(send_workspace_count, (u32 count));
CTF_MOCK_VOID_EXTERN(setup_root);

#endif
