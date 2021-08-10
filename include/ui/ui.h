#pragma once

#include "stm32746g_discovery.h"
#include "mbed_stats.h"
#include <cstdint>

namespace UI {
class Display {
public:
  /**
   * Initialize the display
   */
  void init();
  /**
   * Write a log message to the display
   *
   * @param text Text to write
   */
  void log(uint8_t *text);

  void clear_line(int line_no);

  void update_stats(mbed_stats_cpu_t *stats, int sample_time_ms);
};

} // namespace UI
