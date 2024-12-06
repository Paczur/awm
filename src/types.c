#include "types.h"

const char *awm_status_str[] = {
#define X(x) [AWM_STATUS_##x] = #x,
  AWM_STATUS_X
#undef X
};
