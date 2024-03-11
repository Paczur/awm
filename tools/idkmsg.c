#include <xcb/xcb.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

typedef unsigned int uint;

#define LEN(x) (sizeof(x)/sizeof((x)[0]))

enum ATOMS {
  ATOM_WM_CHANGE_STATE,
  ATOM_NET_CLOSE_WINDOW,
  ATOM_NET_REQUEST_FRAME_EXTENTS
};

char *atom_names[] = {
  "WM_CHANGE_STATE",
  "_NET_CLOSE_WINDOW",
  "_NET_REQUEST_FRAME_EXTENTS"
};

enum CHOICE {
  CHOICE_DEST = 1,
  CHOICE_WINDOW = 2,
};

uint8_t atom_fields[LEN(atom_names)] = {
  CHOICE_WINDOW,
  CHOICE_WINDOW,
  CHOICE_WINDOW,
};

xcb_connection_t *conn;
const xcb_setup_t* setup;
xcb_screen_t* screen;

xcb_atom_t atom;
size_t atom_index = SIZE_MAX;
xcb_window_t target;
xcb_window_t window;
uint data[5];

void print_help(int i) {
  puts("HELP");
  exit(i);
}

xcb_window_t hex_to_window(const char* hex, size_t len) {
  xcb_window_t window = 0;
  uint mul = 1;
  for(size_t i=len-1; i>0; i--) {
    window += mul*
      ((hex[i]>='a') ? hex[i]-'a'+10 :
       (hex[i]>='A') ? hex[i]-'A'+10 :
       hex[i]-'0');
    mul*=16;
  }
  window += mul*
      ((hex[0]>='a') ? hex[0]-'a'+10 :
       (hex[0]>='A') ? hex[0]-'A'+10 :
       hex[0]-'0');
  return window;
}

uint dec_to_uint(const char* dec, size_t len) {
  uint ret = 0;
  uint mul = 1;
  for(size_t i=len-1; i>0; i--) {
    ret += mul*(dec[i]-'0');
    mul*=10;
  }
  ret += mul*(dec[0]-'0');
  return ret;
}

xcb_window_t dec_to_window(const char* dec, size_t len) {
  return (xcb_window_t)dec_to_uint(dec, len);
}

xcb_window_t str_to_window(const char* str) {
  if(str[1] == 'x' || str[1] == 'X')
    return hex_to_window(str+2, strlen(str+2));
  return dec_to_window(str, strlen(str));
}

void send_event(void) {
  xcb_client_message_event_t msg = { XCB_CLIENT_MESSAGE, 32, 0, window, atom,
    .data.data32={ data[0], data[1], data[2], data[3], data[4] } };
  xcb_event_mask_t mask = XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
    XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT;
  xcb_send_event(conn, false, target, mask, (char*)&msg);
  xcb_flush(conn);
  exit(0);
}

int main(int argc, const char* argv[]) {
  int argi = 1;
  xcb_intern_atom_cookie_t cookie;
  xcb_intern_atom_reply_t *reply;
  conn = xcb_connect(NULL, NULL);
  setup = xcb_get_setup(conn);
  screen = xcb_setup_roots_iterator(setup).data;
  if(argc-1 < argi) print_help(1);

  target = screen->root;
  window = screen->root;

  for(size_t i=0; i<LEN(atom_names); i++) {
    if(strcmp(argv[argi], atom_names[i]) == 0) {
      atom_index = i;
      break;
    }
  }
  if(atom_index == SIZE_MAX) print_help(1);
  cookie = xcb_intern_atom(conn, 0, strlen(atom_names[atom_index]),
                           atom_names[atom_index]);
  reply = xcb_intern_atom_reply(conn, cookie, NULL);
  if(reply != NULL) {
    atom = reply->atom;
    free(reply);
  } else {
    print_help(1);
  }
  argi++;
  if(argc-1 < argi) send_event();
  if(atom_fields[atom_index] & CHOICE_DEST &&
     strcmp(argv[argi], "root") != 0) {
    target = str_to_window(argv[argi]);
    argi++;
  } else {
    target = screen->root;
  }
  if(argc-1 < argi) send_event();
  if(atom_fields[atom_index] & CHOICE_WINDOW &&
     strcmp(argv[argi], "root") != 0) {
    window = str_to_window(argv[argi]);
    argi++;
  } else {
    window = screen->root;
  }
  switch(atom_index) {
  case ATOM_WM_CHANGE_STATE:
    data[0] = 3;
    send_event();
  break;
  case ATOM_NET_CLOSE_WINDOW:
    data[0] = XCB_CURRENT_TIME;
    data[1] = 2;
    send_event();
  break;
  case ATOM_NET_REQUEST_FRAME_EXTENTS:
    send_event();
  break;
  }
  if(argc-1 < argi) send_event();
  for(size_t i=0; i<5 && argi<argc; i++, argi++) {
    data[i] = dec_to_uint(argv[argi], strlen(argv[argi]));
  }
  send_event();
  return 0;
}
