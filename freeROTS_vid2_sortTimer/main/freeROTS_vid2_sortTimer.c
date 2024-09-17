
#include <stdio.h>
#include "sdkconfig.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_spi_flash.h"

#include "input_iot.h"
#include "output_iot.h"



#define NUM_TIMERS 2
/* An array to hold handles to the created timers. */
TimerHandle_t xTimers[ NUM_TIMERS ];

/* Declare a variable to hold the created event group. */
EventGroupHandle_t xEventGroup;
#define BIT_EVENT_BUTTON_PRESS	( 1 << 0 )
#define BIT_EVENT_UART_RECV	( 1 << 1 )


void vTask1( void * pvParameters )
{
    for( ;; )
    {
        EventBits_t  uxBits = xEventGroupWaitBits(
                                xEventGroup,   /* The event group being tested. */
                                BIT_EVENT_BUTTON_PRESS | BIT_EVENT_UART_RECV, /* The bits within the event group to wait for. */
                                pdTRUE,        /* BIT_0 & BIT_4 should be cleared before returning. */
                                pdFALSE,       /* Don't wait for both bits, either bit will do. */
                                portMAX_DELAY );/* Wait a maximum of 100ms for either bit to be set. */

        if(  uxBits & BIT_EVENT_BUTTON_PRESS )
        {
            /* xEventGroupWaitBits() returned because both bits were set. */
            printf("Button press\n");
            output_io_toggle(2);
        }
        if(  uxBits & BIT_EVENT_UART_RECV )
        {
            /* xEventGroupWaitBits() returned because just BIT_0 was set. */
            printf("UART DATA\n");
            //....
        }

    }
}
//ham xu ly su kien nut nhan
void button_callback( int pin )
{
    if( pin == GPIO_NUM_4)
    {
        BaseType_t pxHigherPriorityTaskWoken;
        xEventGroupSetBitsFromISR(
                              xEventGroup,   /* The event group being updated. */
                              BIT_EVENT_BUTTON_PRESS , /* The bits being set. */
                              &pxHigherPriorityTaskWoken );
    }
}



void vTimerCallback( TimerHandle_t xTimer )
 {
    uint32_t ulCount;
    configASSERT( xTimer );
    ulCount = ( uint32_t ) pvTimerGetTimerID( xTimer );

    if(ulCount == 0)
    {
        output_io_toggle(2);
    }

    else if(ulCount == 1)
    {
        printf("Hello\n");
    }

 }


void app_main(void)
{
    for( int x = 0; x < NUM_TIMERS; x++ )
     {
         xTimers[ 0 ] = xTimerCreate
                   ( /* Just a text name, not used by the RTOS
                     kernel. */
                     "TimerBlink",
                     /* The timer period in ticks, must be
                     greater than 0. */
                     pdMS_TO_TICKS(500),
                     /* The timers will auto-reload themselves
                     when they expire. */
                     pdTRUE,
                     /* The ID is used to store a count of the
                     number of times the timer has expired, which
                     is initialised to 0. */
                     ( void * ) 0,
                     /* Each timer calls the same callback when
                     it expires. */
                     vTimerCallback
                   );
        xTimers[ 1 ] = xTimerCreate
                   ( /* Just a text name, not used by the RTOS
                     kernel. */
                     "TimerPrintf",
                     /* The timer period in ticks, must be
                     greater than 0. */
                     pdMS_TO_TICKS(500),
                     /* The timers will auto-reload themselves
                     when they expire. */
                     pdTRUE,
                     /* The ID is used to store a count of the
                     number of times the timer has expired, which
                     is initialised to 0. */
                     ( void * ) 1,
                     /* Each timer calls the same callback when
                     it expires. */
                     vTimerCallback
                   );
     }

    // xTimerStart(xTimers[0],0);
    // xTimerStart(xTimers[1],0);



    output_io_create(2);
    input_io_create(4, HI_TO_LO);
    input_set_callback(button_callback);

    xEventGroup = xEventGroupCreate();

    xTaskCreate(
                vTask1,       /* Function that implements the task. */
                "vTask1",          /* Text name for the task. */
                2048,      /* Stack size in words, not bytes. */
                NULL,    /* Parameter passed into the task. */
                4,/* Priority at which the task is created. */
                NULL );      /* Used to pass out the created task's handle. */

}
