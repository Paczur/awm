#ifndef H_AWM_X
#define H_AWM_X

#include "../types.h"

#define X_EVENT_TYPE_KEY_PRESS 2
#define X_EVENT_TYPE_KEY_RELEASE 3
#define X_EVENT_TYPE_BUTTON_PRESS 4
#define X_EVENT_TYPE_BUTTON_RELEASE 5
#define X_EVENT_TYPE_MOTION_NOTIFY 6
#define X_EVENT_TYPE_ENTER_NOTIFY 7
#define X_EVENT_TYPE_LEAVE_NOTIFY 8
#define X_EVENT_TYPE_FOCUS_IN 9
#define X_EVENT_TYPE_FOCUS_OUT 10
#define X_EVENT_TYPE_KEYMAP_NOTIFY 11
#define X_EVENT_TYPE_EXPOSE 12
#define X_EVENT_TYPE_GRAPHICS_EXPOSURE 13
#define X_EVENT_TYPE_NO_EXPOSURE 14
#define X_EVENT_TYPE_VISIBILITY_NOTIFY 15
#define X_EVENT_TYPE_CREATE_NOTIFY 16
#define X_EVENT_TYPE_DESTROY_NOTIFY 17
#define X_EVENT_TYPE_UNMAP_NOTIFY 18
#define X_EVENT_TYPE_MAP_NOTIFY 19
#define X_EVENT_TYPE_MAP_REQUEST 20
#define X_EVENT_TYPE_REPARENT_NOTIFY 21
#define X_EVENT_TYPE_CONFIGURE_NOTIFY 22
#define X_EVENT_TYPE_CONFIGURE_REQUEST 23
#define X_EVENT_TYPE_GRAVITY_NOTIFY 24
#define X_EVENT_TYPE_RESIZE_REQUEST 25
#define X_EVENT_TYPE_CIRCULATE_NOTIFY 26
#define X_EVENT_TYPE_CIRCULATE_REQUEST 27
#define X_EVENT_TYPE_PROPERTY_NOTIFY 28
#define X_EVENT_TYPE_SELECTION_CLEAR 29
#define X_EVENT_TYPE_SELECTION_REQUEST 30
#define X_EVENT_TYPE_SELECTION_NOTIFY 31
#define X_EVENT_TYPE_COLORMAP_NOTIFY 32
#define X_EVENT_TYPE_CLIENT_MESSAGE 33
#define X_EVENT_TYPE_MAPPING_NOTIFY 34

typedef uint32_t x_window;
typedef struct x_event x_event;
typedef struct x_keymap x_keymap;

void x_init(void);
void x_deinit(void);

void x_keyboard_grab(void);
void x_keyboard_ungrab(void);
void x_window_keyboard_grab(x_window);

x_event *x_event_next(x_event *prev);
uint32_t x_event_type(const x_event *);
x_window x_event_window(const x_event *);

x_keymap *x_keymap_get(void);
uint32_t x_keymap_length(x_keymap *);
uint32_t *x_keymap_syms(x_keymap *);
uint8_t x_keymap_syms_per_code(x_keymap *);

void x_window_map(x_window);
void x_window_resize(x_window, struct awm_rect *);

uint8_t x_key_code(x_event *);
uint8_t x_key_mod(x_event *);

void x_free(void *);

#endif
