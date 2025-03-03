#include "syms.h"

static u16 buff[2] = {0, 0};

const u16 *keysym_to_u16(u32 keysym) {
  if(keysym >= KEY_space && keysym <= KEY_asciitilde) {
    buff[0] = (u16)(keysym - KEY_space) + ' ';
    return buff;
  }
  return NULL;
}
