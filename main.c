#define XK_LATIN1 //letters
#define XK_MISCELLANY //modifiers and special
#include <X11/keysymdef.h>
#include <X11/XF86keysym.h>
#include <xcb/xcb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

xcb_connection_t* conn;
const xcb_setup_t* setup;
xcb_screen_t* screen;
xcb_generic_event_t* event;
uint32_t window_id;
uint32_t gc_id;
uint32_t value_mask;
uint32_t* value_list;
xcb_get_keyboard_mapping_reply_t* kmapping;
bool mode = false;
extern char **environ;

void sh(char* cmd) {
  int i=0;
  while(environ[i]!=0) {
    printf("%s\n", environ[i]);
    i++;
  }
  int pid = fork();
  if(pid == 0) {
    execl("/bin/sh", "sh", "-c", cmd, NULL);
  }
}

int main(int argc, char *argv[], char *envp[]) {
  environ = envp;

  /* CONNECT */
  conn =  xcb_connect(NULL, NULL); //display and screen
  if(xcb_connection_has_error(conn)) {
    printf("Error\n");
    return 1;
  }
  setup = xcb_get_setup(conn);
  screen = xcb_setup_roots_iterator(setup).data;

  /* CREATE WINDOW */
  value_list = malloc(32*2); //Max 2 values
  value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  value_list[0] = screen->black_pixel;
  value_list[1] = XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_EXPOSURE;
  window_id = xcb_generate_id(conn);
  xcb_create_window(conn,
                    24, //depth
                    window_id,
                    screen->root,
                    400, //x
                    400, //y
                    200, //width
                    200, //height
                    5, //border_width
                    XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    screen->root_visual,
                    value_mask,
                    value_list);
  xcb_map_window(conn, window_id);
  xcb_flush(conn);

  /* CREATE DRAWING CONTEXT */
  gc_id = xcb_generate_id(conn);
  value_mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
  value_list[0] = screen->white_pixel;
  value_list[1] = 0;
  xcb_create_gc(conn,
                gc_id,
                screen->root,
                value_mask,
                value_list);
  free(value_list);
  xcb_rectangle_t rect = {16, 30, 20, 40};

  /* HANDLE EVENTS */
  while((event = xcb_wait_for_event(conn))) {
    switch(event->response_type) {
    case XCB_KEY_PRESS:
      xcb_get_keyboard_mapping_cookie_t cookie =
        xcb_get_keyboard_mapping(conn, setup->min_keycode,
                                 setup->max_keycode-setup->min_keycode);
      kmapping = xcb_get_keyboard_mapping_reply(conn, cookie, NULL);
      xcb_keycode_t keycode = ((xcb_key_press_event_t*)event)->detail;
      xcb_keysym_t* keysyms = xcb_get_keyboard_mapping_keysyms(kmapping);
      xcb_keysym_t keysym = keysyms[(keycode-setup->min_keycode)*
        kmapping->keysyms_per_keycode];
      switch(keysym) {
      case XK_Super_L:
        mode = !mode;
      break;
      case XK_Escape:
        mode = false;
      break;
      case XK_Return:
        sh("mlterm");
        printf("mode state: %d\n", mode);
      break;
      }
    break;
    case XCB_EXPOSE:
      printf("Exposure\n");
      xcb_poly_fill_rectangle(conn,
                              window_id,
                              gc_id,
                              1,
                              &rect);
    break;
    default:
      printf("Not supported event\n");
    break;
    }
    xcb_flush(conn);
  }
  return 0;
}
