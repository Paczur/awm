#include "layout.h"

#include <layout/layout.h>

#include "../mocks/mocks.h"

CTF_TEST_STATIC(map_request_on) {
  struct geometry monitor = {1, 1, 1920, 1080};
  init_layout(&monitor, 1, 0);
  subtest(first_window_in_a_workspace) {
    subtest(configures_it_to_fullscreen_maps_it_and_sends)
      mock_select(configure_window) {
      mock_expect_nth(1, x, ==, monitor.x);
      mock_expect_nth(1, y, ==, monitor.y);
      mock_expect_nth(1, width, ==, monitor.width);
      mock_expect_nth(1, height, ==, monitor.height);
      mock_select(map_window) {
        mock_select(send_workspace) {
          map_request(1);
          expect(mock_call_count, ==, 1);
        }
        expect(mock_call_count, ==, 1);
      }
      expect(mock_call_count, ==, 1);
    }
  }
  subtest(second_window_in_a_workspace) {
    subtest(
      configures_them_to_vertical_slices_new_one_on_right_maps_it_and_sends)
      mock_select(configure_window) {
      mock_expect_nth(1, window, ==, 1);
      mock_expect_nth(1, x, ==, monitor.x);
      mock_expect_nth(1, y, ==, monitor.y);
      mock_expect_nth(1, width, ==, monitor.width / 2);
      mock_expect_nth(1, height, ==, monitor.height);
      mock_expect_nth(2, window, ==, 2);
      mock_expect_nth(2, x, ==, monitor.x + monitor.width / 2);
      mock_expect_nth(2, y, ==, monitor.y);
      mock_expect_nth(2, width, ==, monitor.width / 2);
      mock_expect_nth(2, height, ==, monitor.height);
      mock_select(map_window) {
        mock_select(send_workspace) {
          map_request(2);
          expect(mock_call_count, ==, 1);
        }
        expect(mock_call_count, ==, 1);
      }
      expect(mock_call_count, ==, 2);
    }
  }
  subtest(third_window_in_a_workspace) {
    subtest(configures_first_to_vertical_slice_second_and_third_to_quarters_
              maps_it_and_sends) mock_select(configure_window) {
      mock_expect_nth(1, window, ==, 1);
      mock_expect_nth(1, x, ==, monitor.x);
      mock_expect_nth(1, y, ==, monitor.y);
      mock_expect_nth(1, width, ==, monitor.width / 2);
      mock_expect_nth(1, height, ==, monitor.height);

      mock_expect_nth(2, window, ==, 2);
      mock_expect_nth(2, x, ==, monitor.x + monitor.width / 2);
      mock_expect_nth(2, y, ==, monitor.y);
      mock_expect_nth(2, width, ==, monitor.width / 2);
      mock_expect_nth(2, height, ==, monitor.height / 2);

      mock_expect_nth(3, window, ==, 4);
      mock_expect_nth(3, x, ==, monitor.x + monitor.width / 2);
      mock_expect_nth(3, y, ==, monitor.y + monitor.height / 2);
      mock_expect_nth(3, width, ==, monitor.width / 2);
      mock_expect_nth(3, height, ==, monitor.height / 2);
      mock_select(map_window) {
        mock_select(send_workspace) {
          map_request(4);
          expect(mock_call_count, ==, 1);
        }
        expect(mock_call_count, ==, 1);
      }
      expect(mock_call_count, ==, 3);
    }
  }
  subtest(fourth_window_in_a_workspace) {
    subtest(configures_all_windows_to_quarters_maps_it_and_sends)
      mock_select(configure_window) {
      mock_expect_nth(1, window, ==, 1);
      mock_expect_nth(1, x, ==, monitor.x);
      mock_expect_nth(1, y, ==, monitor.y);
      mock_expect_nth(2, window, ==, 2);
      mock_expect_nth(2, x, ==, monitor.x + monitor.width / 2);
      mock_expect_nth(2, y, ==, monitor.y);
      mock_expect_nth(3, window, ==, 3);
      mock_expect_nth(3, x, ==, monitor.x);
      mock_expect_nth(3, y, ==, monitor.y + monitor.height / 2);
      mock_expect_nth(4, window, ==, 4);
      mock_expect_nth(4, x, ==, monitor.x + monitor.width / 2);
      mock_expect_nth(4, y, ==, monitor.y + monitor.height / 2);
      mock_expect(width, ==, monitor.width / 2);
      mock_expect(height, ==, monitor.height / 2);
      mock_select(map_window) {
        mock_select(send_workspace) {
          map_request(3);
          expect(mock_call_count, ==, 1);
        }
        expect(mock_call_count, ==, 1);
      }
      expect(mock_call_count, ==, 4);
    }
  }
}

CTF_TEST_STATIC(unmap_notify_in_order_with) {
  struct geometry monitor = {1, 1, 1920, 1080};
  init_layout(&monitor, 1, 0);
  map_request(1);
  map_request(2);
  map_request(3);
  map_request(4);
  subtest(four_windows_in_a_workspace) {
    subtest(configures_first_to_vertical_slice_second_and_third_to_quarters_
              and_sends_workspace) mock_select(configure_window) {
      mock_expect_nth(1, window, ==, 1);
      mock_expect_nth(1, x, ==, monitor.x);
      mock_expect_nth(1, y, ==, monitor.y);
      mock_expect_nth(1, width, ==, monitor.width / 2);
      mock_expect_nth(1, height, ==, monitor.height);

      mock_expect_nth(2, window, ==, 2);
      mock_expect_nth(2, x, ==, monitor.x + monitor.width / 2);
      mock_expect_nth(2, y, ==, monitor.y);
      mock_expect_nth(2, width, ==, monitor.width / 2);
      mock_expect_nth(2, height, ==, monitor.height / 2);

      mock_expect_nth(3, window, ==, 4);
      mock_expect_nth(3, x, ==, monitor.x + monitor.width / 2);
      mock_expect_nth(3, y, ==, monitor.y + monitor.height / 2);
      mock_expect_nth(3, width, ==, monitor.width / 2);
      mock_expect_nth(3, height, ==, monitor.height / 2);
      mock_select(send_workspace) {
        unmap_notify(3);
        expect(mock_call_count, ==, 1);
      }
      expect(mock_call_count, ==, 3);
    }
  }
  subtest(three_windows_in_a_workspace) {
    subtest(
      configures_them_to_vertical_slices_new_one_on_right_maps_it_and_sends)
      mock_select(configure_window) {
      mock_expect_nth(1, window, ==, 1);
      mock_expect_nth(1, x, ==, monitor.x);
      mock_expect_nth(1, y, ==, monitor.y);
      mock_expect_nth(1, width, ==, monitor.width / 2);
      mock_expect_nth(1, height, ==, monitor.height);
      mock_expect_nth(2, window, ==, 2);
      mock_expect_nth(2, x, ==, monitor.x + monitor.width / 2);
      mock_expect_nth(2, y, ==, monitor.y);
      mock_expect_nth(2, width, ==, monitor.width / 2);
      mock_expect_nth(2, height, ==, monitor.height);
      mock_select(send_workspace) {
        unmap_notify(4);
        expect(mock_call_count, ==, 1);
      }
      expect(mock_call_count, ==, 2);
    }
  }
  subtest(two_windows_in_a_workspace) {
    subtest(configures_it_to_fullscreen_maps_it_and_sends)
      mock_select(configure_window) {
      mock_expect_nth(1, window, ==, 1);
      mock_expect_nth(1, x, ==, monitor.x);
      mock_expect_nth(1, y, ==, monitor.y);
      mock_expect_nth(1, width, ==, monitor.width);
      mock_expect_nth(1, height, ==, monitor.height);
      mock_select(send_workspace) {
        unmap_notify(2);
        expect(mock_call_count, ==, 1);
      }
      expect(mock_call_count, ==, 1);
    }
  }
}

CTF_GROUP(layout_spec) = {
  map_request_on,
  unmap_notify_in_order_with,
};

CTF_GROUP_SETUP(layout_spec) { mock_group(layout_x_mocks); }
