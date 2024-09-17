#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "input_iot.h"
#include "output_iot.h"

#define BLINK_GPIO 2 //CONFIG_BLINK_GPIO

void input_event_callback(int pin)
{
    if(pin == GPIO_NUM_4)
    {
        output_io_toggle(BLINK_GPIO);
    }
}

void app_main(void)
{
    output_io_create(BLINK_GPIO);

    input_io_create(GPIO_NUM_4, HI_TO_LO);
    input_set_callback(input_event_callback);
}