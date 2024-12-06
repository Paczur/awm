#include <stdio.h>

#include "log/log.h"

const char *awm_current_component = "AWM";
int awm_component_bar = 0;

int main(void) {
  log(LOG_LEVEL_ERROR, "awm entry point");
  return 0;
}
