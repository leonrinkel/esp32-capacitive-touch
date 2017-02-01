#include "freertos/FreeRTOS.h"

#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"

#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"

#include "nvs_flash.h"

#include "driver/touch_pad.h"

#define TOUCH_THRESHOLD 200

const static char *TAG = "touch";

esp_err_t event_handler(void *ctx, system_event_t *event) {
  return ESP_OK;
}

const touch_pad_t pads[] = {
  TOUCH_PAD_NUM0,
  //TOUCH_PAD_NUM1, does not work on sparkfun thing because of button and pullup
  TOUCH_PAD_NUM2,
  TOUCH_PAD_NUM3,
  TOUCH_PAD_NUM4,
  TOUCH_PAD_NUM5,
  TOUCH_PAD_NUM6,
  TOUCH_PAD_NUM7,
  TOUCH_PAD_NUM8,
  TOUCH_PAD_NUM9
};

void rtc_intr(void *arg) {
  uint32_t pad_intr = READ_PERI_REG(SENS_SAR_TOUCH_CTRL2_REG) & 0x3ff;
  uint32_t rtc_intr = READ_PERI_REG(RTC_CNTL_INT_ST_REG);
  uint8_t i = 0;

  // clear interrupt
  WRITE_PERI_REG(RTC_CNTL_INT_CLR_REG, rtc_intr);
  SET_PERI_REG_MASK(SENS_SAR_TOUCH_CTRL2_REG, SENS_TOUCH_MEAS_EN_CLR);

  if (rtc_intr & RTC_CNTL_TOUCH_INT_ST) {
    for (i = 0; i < 10; ++i) {
      if ((pad_intr >> i) & 0x01) {
        ESP_LOGI(TAG, "touched");
      }
    }
  }
}

void app_main(void) {
  nvs_flash_init();

  touch_pad_init();

  uint16_t value;
  for(int i = 0; i < sizeof(pads) / sizeof(touch_pad_t) ; i++) {
    // calibration
    touch_pad_read(pads[i], &value);
    touch_pad_config(pads[i], value - TOUCH_THRESHOLD);

    touch_pad_isr_handler_register(rtc_intr, NULL, 0, NULL);
  }
}
