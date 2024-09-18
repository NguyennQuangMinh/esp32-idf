#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "input_iot.h"
    
#define BLINK_GPIO 2 //CONFIG_BLINK_GPIO

void input_event_callback(int pin)
{
    if(pin == GPIO_NUM_0)
    {
        static int x = 0;
        gpio_set_level(BLINK_GPIO, x);
        x = 1 - x;
    }
}

void app_main(void)
{
    gpio_pad_select_gpio(BLINK_GPIO);

    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    input_io_create(GPIO_NUM_0, HI_TO_LO);

    input_set_callback(input_event_callback);
}
