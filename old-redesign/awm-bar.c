#include <stdio.h>

#include "log/log.h"

const char *awm_current_component = "BAR";
int awm_component_bar = 1;

int main(void) {
  log(LOG_LEVEL_ERROR, "awm bar entry point");
  return 0;
}
