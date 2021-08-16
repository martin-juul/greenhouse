#include "ui/ui.h"
#include "Queue.h"
#include "mbed.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32f7xx_hal.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <queue>
#include <string>
#include <vector>

using namespace UI;

static const int MAX_LOG_LINES = 6;
static const int LOG_Y_POS = 13;
int log_idx = 0;
uint64_t prev_idle_time = 0;

void set_default_font() {
  BSP_LCD_SetFont(&Font12);
}

void Display::clear_line(int line_no) {
  BSP_LCD_DisplayStringAt(0, LINE(line_no), (uint8_t *)"                                              ", LEFT_MODE);
  BSP_LCD_DisplayStringAt(0, LINE(line_no), (uint8_t *)"                                              ", CENTER_MODE);
  BSP_LCD_DisplayStringAt(0, LINE(line_no), (uint8_t *)"                                              ", RIGHT_MODE);

  // this doesn't appear to work properly?
  BSP_LCD_ClearStringLine(LINE(line_no));
}

void Display::init() {
  BSP_LCD_Init();
  BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, LCD_FB_START_ADDRESS);
  BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);

  BSP_LCD_SetFont(&Font16);
  BSP_LCD_Clear(LCD_COLOR_BLACK);
  BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

  BSP_LCD_DisplayStringAt(2, 2, (uint8_t *)"greenhouse (server)", LEFT_MODE);
  BSP_LCD_DrawHLine(0, 20, 480);

  set_default_font();

  BSP_LCD_DrawHLine(0, LINE(LOG_Y_POS - 1) - 1, 480);
  BSP_LCD_DisplayStringAt(10, LINE(LOG_Y_POS) - 12, (uint8_t *)"console", LEFT_MODE);
  BSP_LCD_DrawHLine(0, LINE(LOG_Y_POS), 480);
}

void Display::log(uint8_t *text) {
  if (log_idx > MAX_LOG_LINES) {
    // Clear display
    for (int i = 1; i < 10; i++) {
      clear_line(LINE(i + LOG_Y_POS));
    }

    log_idx = 0;
  }

  ++log_idx;

  BSP_LCD_DisplayStringAt(10, LINE(log_idx + LOG_Y_POS), text, LEFT_MODE);
}

string microsecond_to_sec_str(uint64_t us) {
  uint64_t converted;

  if (us > 1) {
    converted = us / 1000 / 1000;
  } else {
    converted = 0; 
  }

  return std::to_string(converted);
}

string make_stat_str(string label, string suffix, uint64_t us) {
  string stat = string(label);
  stat.append(microsecond_to_sec_str(us));
  stat.append(suffix);

  return stat;
}

void Display::update_stats(mbed_stats_cpu_t *stats, int sample_time_ms) {
  clear_line(3);
  clear_line(4);

  string uptime = make_stat_str("Uptime: ", "s", stats->uptime);
  string idle_time = make_stat_str("Idle: ", "s", stats->idle_time);
  string sleep = make_stat_str("Sleep: ", "s", stats->sleep_time);
  string deep_sleep = make_stat_str("Deep sleep: ", "s", stats->deep_sleep_time);

  BSP_LCD_DisplayStringAt(25, LINE(3), (uint8_t *)uptime.c_str(), LEFT_MODE);
  BSP_LCD_DisplayStringAt(120, LINE(3), (uint8_t *)idle_time.c_str(), LEFT_MODE);
  BSP_LCD_DisplayStringAt(205, LINE(3), (uint8_t *)sleep.c_str(), LEFT_MODE);
  BSP_LCD_DisplayStringAt(300, LINE(3), (uint8_t *)deep_sleep.c_str(), LEFT_MODE);

  uint64_t diff_usec = (stats->idle_time - prev_idle_time);
  uint8_t idle = (diff_usec * 100) / (sample_time_ms*1000);
  uint8_t usage = 100 - ((diff_usec * 100) / (sample_time_ms*1000));
  prev_idle_time = stats->idle_time;

  string usage_str = string("Usage: ");
  usage_str.append(std::to_string(usage));
  usage_str.append("%");
  
  BSP_LCD_DisplayStringAt(25, LINE(4), (uint8_t *)usage_str.c_str(), LEFT_MODE);
}
