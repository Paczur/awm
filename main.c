#define XK_LATIN1 //letters
#define XK_MISCELLANY //modifiers and special
#include "main.h"
#include <xcb/randr.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define VALUE_LIST_SIZE 10
#define WINDOW_HEIGHT 400
#define WINDOW_WIDTH 600

xcb_connection_t* conn;
const xcb_setup_t* setup;
xcb_screen_t* screen;
xcb_generic_event_t* event;
xcb_keysym_t* keysyms;
xcb_get_keyboard_mapping_reply_t *kmapping;
uint32_t window_id;
uint32_t gc_id;
bool mode = false;
extern char **environ;

void sh(char* cmd) {
  int pid = fork();
  if(pid == 0) {
    execl("/bin/sh", "sh", "-c", cmd, NULL);
  }
}

xcb_keycode_t keysym_to_keycode(xcb_keysym_t keysym) {
  for(int i=setup->min_keycode; i<setup->max_keycode; i++) {
    if(keycode_to_keysym(i) == keysym)
      return i;
  }
  return -1;
}

xcb_keysym_t keycode_to_keysym(xcb_keycode_t keycode) {
  return keysyms[(keycode-setup->min_keycode) * kmapping->keysyms_per_keycode];
}

void setup_wm(void) {
  conn = xcb_connect(NULL, NULL);
  setup = xcb_get_setup(conn);
  screen = xcb_setup_roots_iterator(setup).data;

  xcb_randr_get_screen_resources_cookie_t randr_cookie =
    xcb_randr_get_screen_resources(conn, screen->root);

  xcb_get_keyboard_mapping_cookie_t kmap_cookie =
    xcb_get_keyboard_mapping(conn, setup->min_keycode,
                             setup->max_keycode-setup->min_keycode);

  uint32_t values = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
    XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
    XCB_EVENT_MASK_STRUCTURE_NOTIFY |
    XCB_EVENT_MASK_PROPERTY_CHANGE;
  xcb_change_window_attributes_checked(conn, screen->root,
                                       XCB_CW_EVENT_MASK, &values);
  xcb_ungrab_key(conn, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);

  kmapping = xcb_get_keyboard_mapping_reply(conn, kmap_cookie, NULL);
  keysyms = xcb_get_keyboard_mapping_keysyms(kmapping);

  xcb_grab_key(conn, 1, screen->root, XCB_MOD_MASK_ANY,
               keysym_to_keycode(XK_Super_L),
               XCB_GRAB_MODE_ASYNC,
               XCB_GRAB_MODE_ASYNC);

  xcb_randr_get_screen_resources_reply_t *screen_res =
    xcb_randr_get_screen_resources_reply(conn, randr_cookie, 0);

  int crtcs_num = xcb_randr_get_screen_resources_crtcs_length(screen_res);
  printf("%d\n", crtcs_num);
  xcb_randr_crtc_t *firstCrtc = xcb_randr_get_screen_resources_crtcs(screen_res);

  xcb_randr_get_crtc_info_cookie_t *randr_cookies =
    malloc(crtcs_num*sizeof(xcb_randr_get_crtc_info_cookie_t));

  for(int i=0; i<crtcs_num; i++)
    randr_cookies[i] = xcb_randr_get_crtc_info(conn, *(firstCrtc+i), 0);

  xcb_randr_get_crtc_info_reply_t **randr_crtcs =
    malloc(crtcs_num*sizeof(xcb_randr_get_crtc_info_reply_t));

  for(int i=0; i<crtcs_num; i++) {
    randr_crtcs[i] = xcb_randr_get_crtc_info_reply(conn, randr_cookies[i], 0);
  }
  free(randr_cookies);

  for(int i=0; i<crtcs_num; i++) {
   printf("CRTC[%i] INFO:\n", i);
   printf("x-off\t: %i\n", randr_crtcs[i]->x);
   printf("y-off\t: %i\n", randr_crtcs[i]->y);
   printf("width\t: %i\n", randr_crtcs[i]->width);
   printf("height\t: %i\n\n", randr_crtcs[i]->height);
   free(randr_crtcs[i]);
  }

  fflush(stdout);
  free(screen_res);
  free(randr_crtcs);
  xcb_flush(conn);
}

int main(int argc, char *argv[], char *envp[]) {
  environ = envp;

  setup_wm();
  printf("%dx%d\n", screen->width_in_pixels,
         screen->height_in_pixels);
  printf("%dx%d\n", screen->width_in_pixels/2 - WINDOW_HEIGHT/2,
         screen->width_in_pixels/2 - WINDOW_HEIGHT/2);
  while((event = xcb_wait_for_event(conn))) {
    switch(event->response_type) {
    case XCB_KEY_PRESS:
      xcb_keycode_t keycode = ((xcb_key_press_event_t*)event)->detail;
      xcb_keysym_t keysym = keycode_to_keysym(keycode);
      if(mode) {
        switch(keysym) {
        case XK_Escape:
          mode = false;
          xcb_ungrab_key(conn, keysym_to_keycode(XK_Return),
                         screen->root, XCB_MOD_MASK_ANY);
          xcb_ungrab_key(conn, keysym_to_keycode(XK_Escape),
                         screen->root, XCB_MOD_MASK_ANY);
        break;
        case XK_Return:
          sh("mlterm");
        break;
        }
      } else if(keysym == XK_Super_L) {
        mode = true;
        xcb_grab_key(conn, 1, screen->root, XCB_MOD_MASK_ANY,
                     keysym_to_keycode(XK_Return),
                     XCB_GRAB_MODE_ASYNC,
                     XCB_GRAB_MODE_ASYNC);
        xcb_grab_key(conn, 1, screen->root, XCB_MOD_MASK_ANY,
                     keysym_to_keycode(XK_Escape),
                     XCB_GRAB_MODE_ASYNC,
                     XCB_GRAB_MODE_ASYNC);
      }
    break;
    case XCB_MAP_REQUEST:
      xcb_map_request_event_t *e = (xcb_map_request_event_t *) event;
      xcb_map_window(conn, e->window);
      uint32_t vals[5];
      vals[0] = screen->width_in_pixels/2 - WINDOW_WIDTH/2; //x
      vals[1] = screen->width_in_pixels/2 - WINDOW_HEIGHT/2;//y
      vals[2] = WINDOW_WIDTH; //width
      vals[3] = WINDOW_HEIGHT; //height
      vals[4] = 0; //border_width
      xcb_configure_window(conn, e->window, XCB_CONFIG_WINDOW_X |
                           XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
                           XCB_CONFIG_WINDOW_HEIGHT | XCB_CONFIG_WINDOW_BORDER_WIDTH, vals);
    break;
    default:
      printf ("Unknown event: %"PRIu8"\n", event->response_type);
    break;
    }
    xcb_flush(conn);
  }
  return 0;
}
