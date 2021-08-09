#include "ui.h"
#include "mbed.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32f7xx_hal.h"
#include <cstdint>
#include <cstdio>

int logIdx = 0;

void UI::init() {
  BSP_LCD_Init();
  BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, LCD_FB_START_ADDRESS);
  BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);

  BSP_LCD_SetFont(&Font16);
  BSP_LCD_Clear(LCD_COLOR_BLACK);
  BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

  BSP_LCD_DisplayStringAt(2, 2, (uint8_t *)"greenhouse", LEFT_MODE);
  BSP_LCD_DrawHLine(0, 20, 480);

  BSP_LCD_SetFont(&Font12);
  HAL_Delay(1000);
}

void UI::log(uint8_t *text) {
  if (logIdx > 5) {
    for (int i = 1; i < 6; i++) {
      BSP_LCD_ClearStringLine(LINE(i + 2));
    }

    logIdx = 0;
  }
  
  ++logIdx;

  BSP_LCD_DisplayStringAt(10, LINE(logIdx + 2), text, LEFT_MODE);
}