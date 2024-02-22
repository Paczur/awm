#include "controller.h"

extern char **environ;

int main(int argc, char *argv[], char *envp[]) {
  (void)argc;
  (void)argv;
  environ = envp;
  c_init();
  c_deinit();
  return 0;
}
