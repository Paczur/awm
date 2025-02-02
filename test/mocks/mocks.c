#include "mocks.h"

CTF_MOCK_VOID(grab_keyboard) {}

CTF_MOCK_VOID(ungrab_keyboard) {}

CTF_MOCK_VOID_RET(grab_key, (u8 key, u8 mod), (key, mod)) {
  if(mock_in) {
    mock_check(key);
    mock_check(mod);
  }
}

CTF_MOCK_VOID_RET(send_mode, (u8 mode), (mode)) {
  if(mock_in) mock_check(mode);
}

CTF_MOCK_VOID_ARG(u8, query_mode) {
  if(mock_out) mock_check(mock_return_value);
  return 0;
}

CTF_MOCK_GROUP(shortcut_x_mocks) = {
  CTF_MOCK_BIND(send_mode, NULL),     CTF_MOCK_BIND(query_mode, NULL),
  CTF_MOCK_BIND(grab_keyboard, NULL), CTF_MOCK_BIND(ungrab_keyboard, NULL),
  CTF_MOCK_BIND(grab_key, NULL),
};

CTF_MOCK_VOID_RET(set_mode, (u8 mode), (mode)) {
  if(mock_in) mock_check(mode);
}
