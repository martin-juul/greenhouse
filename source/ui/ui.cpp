#include "ui/ui.h"
#include "Queue.h"
#include "mbed.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32f7xx_hal.h"
#include <cstdint>
#include <cstdio>
#include <queue>
#include <vector>

using namespace UI;

static const int MAX_LOG_LINES = 10;
static const int LOG_Y_POS = 13;
int log_idx = 0;

void set_default_font() {
  BSP_LCD_SetFont(&Font12);
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
      BSP_LCD_DisplayStringAt(10, LINE(i + LOG_Y_POS),
                              (uint8_t *)"                                   ",
                              LEFT_MODE);
    }

    log_idx = 0;
  }

  ++log_idx;

  BSP_LCD_DisplayStringAt(10, LINE(log_idx + LOG_Y_POS), text, LEFT_MODE);
}
