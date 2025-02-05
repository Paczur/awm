#include "layout.h"

#include <layout/layout.h>

#include "../mocks/mocks.h"

CTF_GROUP(layout_spec) = {};

CTF_GROUP_SETUP(layout_spec) { mock_group(layout_x_mocks); }
