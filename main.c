#include <xcb/xcb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

xcb_connection_t* conn;
const xcb_setup_t* setup;
xcb_screen_t* screen;
xcb_generic_event_t* event;
uint32_t window_id;
uint32_t gc_id;
uint32_t value_mask;
uint32_t* value_list;

int main(int argc, char *argv[]) {
  /* CONNECT */
  conn =  xcb_connect(NULL, NULL); //display and screen
  if(xcb_connection_has_error(conn)) {
    printf("Error\n");
    return 1;
  }
  setup = xcb_get_setup(conn);
  screen = xcb_setup_roots_iterator(setup).data;

  /* CREATE WINDOW */
  value_list = malloc(32*2);
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
      printf("Button pressed\n");
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
