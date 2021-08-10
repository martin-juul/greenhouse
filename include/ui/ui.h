#pragma once

#include "stm32746g_discovery.h"
#include "mbed_stats.h"
#include <cstdint>

namespace UI {
class Display {
public:
  /**
   * Initialize the display
   *
   * @retval void
   */
  void init();
  /**
   * Write a log message to the display
   *
   * @param text Text to write
   *
   * @retval void
   */
  void log(uint8_t *text);

  /**
    * Clears a line on the display
    *
    * @retval void
    */
  void clear_line(int line_no);

  /**
    * Update stats display
    *
    * @param stats pointer to mbed_stats_cpu_t
    * @param sample_time_ms Polling time in milliseconds. Used for calculating usage percentage.
    *
    * @retval void
    */
  void update_stats(mbed_stats_cpu_t *stats, int sample_time_ms);
};

} // namespace UI
