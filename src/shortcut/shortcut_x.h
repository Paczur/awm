#ifndef H_AWM_SHORTCUT_X
#define H_AWM_SHORTCUT_X

#include "../types.h"

void delete_sent_shortcut_data(void);
u8 query_mode(void);
void send_mode(u8 mode);
void ungrab_keyboard(void);
void grab_keyboard(void);
void grab_key(u8 key, u8 mod);

#endif
