#include <stdio.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include "input_iot.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"


input_callback_t input_callback = NULL ;
input_ButtonPress timeoutButton_callback = NULL;
static uint64_t _start, _stop , pressTick;
static TimerHandle_t xTimers[1 /*số lượng timer*/];

static void IRAM_ATTR gpio_input_handler(void*arg){
    int gpio_num = (uint32_t) arg;
    uint32_t rtc = xTaskGetTickCountFromISR(); //ms
    if(gpio_get_level(gpio_num) == 0) // bat dau nhan
    {
        _start = rtc ;
        xTimerStart(xTimers[0],0);
    }
    else  // tha tay ra
    {
        xTimerStop(xTimers[0],0);
        _stop = rtc ;
        pressTick = _stop - _start;
        input_callback(pressTick, gpio_num);
        
    }
}

static void vTimerCallback( TimerHandle_t xTimer )
 {
    uint32_t ID ;
    configASSERT(xTimer);
    ID = (uint32_t) pvTimerGetTimerID(xTimer);
    if( ID == 0 )
    {
        timeoutButton_callback(Button_18);
    }

 }

void input_io_create(gpio_num_t gpio_num, interrupt_type_edle_t type)
{
    gpio_pad_select_gpio(gpio_num);
    gpio_set_direction(gpio_num, GPIO_MODE_INPUT);
    gpio_set_pull_mode(gpio_num, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(gpio_num, type);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(gpio_num, gpio_input_handler, (void *)gpio_num);

    xTimers[0] = xTimerCreate
                   ( /* Just a text name, not used by the RTOS
                     kernel. */
                     "TimerForTimeOut",
                     /* The timer period in ticks, must be
                     greater than 0. */
                     pdMS_TO_TICKS(5000),
                     /* The timers will auto-reload themselves
                     when they expire. */
                     pdFALSE,
                     /* The ID is used to store a count of the
                     number of times the timer has expired, which
                     is initialised to 0. */
                     ( void * ) 0,
                     /* Each timer calls the same callback when
                     it expires. */
                     vTimerCallback
                   );
}
uint8_t input_io_get_level(gpio_num_t gpio_num)
{
    return gpio_get_level(gpio_num);
}

void input_set_callback(void *cb)
{
    input_callback = cb;
}
void input_set_timeout_callback(void *cb)
{
    timeoutButton_callback = cb;
}