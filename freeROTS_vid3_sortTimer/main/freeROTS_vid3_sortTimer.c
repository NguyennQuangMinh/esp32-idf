
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


/* Declare a variable to hold the created event group. */ // có khoảng 13 sự kiện trong 1 group nên thoải mái, thiếu tạo thêm
EventGroupHandle_t xEventGroup;
#define BIT_EVENT_BUTTON_PRESS_SHORT	( 1 << 0 )
#define BIT_EVENT_BUTTON_PRESS_NORMAL	( 1 << 1 )
#define BIT_EVENT_BUTTON_PRESS_LONG	( 1 << 2 )
#define BIT_EVENT_UART_RECV	( 1 << 3 )

void vTask1( void * pvParameters )
{
    for( ;; )
    {
        EventBits_t  uxBits = xEventGroupWaitBits(
                                xEventGroup,   /* The event group being tested. */
                                BIT_EVENT_BUTTON_PRESS_SHORT|BIT_EVENT_BUTTON_PRESS_NORMAL | BIT_EVENT_BUTTON_PRESS_LONG| BIT_EVENT_UART_RECV, /* The bits within the event group to wait for. */
                                pdTRUE,        /* BIT_0 & BIT_4 should be cleared before returning. */ // có clear khi chạy qua không
                                pdFALSE,       /* Don't wait for both bits, either bit will do. */  // có đợi tất cả không (False là không)
                                portMAX_DELAY );/* Wait a maximum of 100ms for either bit to be set. */

        if(  uxBits & BIT_EVENT_BUTTON_PRESS_SHORT )
        {
            /* xEventGroupWaitBits() returned because both bits were set. */
            printf("Short press\n");
            
        }
        else if(  uxBits & BIT_EVENT_BUTTON_PRESS_NORMAL)
        {
            /* xEventGroupWaitBits() returned because both bits were set. */
            printf("Normal press\n");
            
        }
        if(  uxBits & BIT_EVENT_BUTTON_PRESS_LONG )
        {
            /* xEventGroupWaitBits() returned because both bits were set. */
            // printf("Long press\n");
            
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
void button_callback( uint64_t tick, int pin )
{
    if( pin == GPIO_NUM_18)
    {
        BaseType_t pxHigherPriorityTaskWoken;
        int press_ms = tick*portTICK_PERIOD_MS;
        if(press_ms <= 1000)
        {
            // press short
            xEventGroupSetBitsFromISR(
                            xEventGroup,   /* The event group being updated. */
                            BIT_EVENT_BUTTON_PRESS_SHORT , /* The bits being set. */
                            &pxHigherPriorityTaskWoken );
        }
        else if(press_ms > 1000 && press_ms < 3000)
        {
            // normal press
            xEventGroupSetBitsFromISR(
                            xEventGroup,   /* The event group being updated. */
                            BIT_EVENT_BUTTON_PRESS_NORMAL , /* The bits being set. */
                            &pxHigherPriorityTaskWoken );
        }
        else if(press_ms >= 3000)
        {
            
        }

    }
}



// static void vTimerCallback( TimerHandle_t xTimer )
//  {
//     uint32_t ID ;
//     configASSERT(xTimer);
//     ID = (uint32_t) pvTimerGetTimerID(xTimer);
//     if( ID == 0 )
//     {

//     }

//  }

void button_timeout_callback(int pin) //ham goi su kien nut nhan
{
    if(pin == GPIO_NUM_18)
    {
        printf("time out\n");
    }
}

void app_main(void)
{
    // for( int x = 0; x < NUM_TIMERS; x++ )
    //  {
    //      xTimers[ 0 ] = xTimerCreate
    //                ( /* Just a text name, not used by the RTOS
    //                  kernel. */
    //                  "TimerBlink",
    //                  /* The timer period in ticks, must be
    //                  greater than 0. */
    //                  pdMS_TO_TICKS(500),
    //                  /* The timers will auto-reload themselves
    //                  when they expire. */
    //                  pdTRUE,
    //                  /* The ID is used to store a count of the
    //                  number of times the timer has expired, which
    //                  is initialised to 0. */
    //                  ( void * ) 0,
    //                  /* Each timer calls the same callback when
    //                  it expires. */
    //                  vTimerCallback
    //                );
    //     // xTimers[ 1 ] = xTimerCreate
    //     //            ( /* Just a text name, not used by the RTOS
    //     //              kernel. */
    //     //              "TimerPrintf",
    //     //              /* The timer period in ticks, must be
    //     //              greater than 0. */
    //     //              pdMS_TO_TICKS(500),
    //     //              /* The timers will auto-reload themselves
    //     //              when they expire. */
    //     //              pdTRUE,
    //     //              /* The ID is used to store a count of the
    //     //              number of times the timer has expired, which
    //     //              is initialised to 0. */
    //     //              ( void * ) 1,
    //     //              /* Each timer calls the same callback when
    //     //              it expires. */
    //     //              vTimerCallback
    //     //            );
    //  }

    // xTimerStart(xTimers[0],0);
    // xTimerStart(xTimers[1],0);



    output_io_create(2);
    input_io_create(18, ANY_EDLE);
    input_set_callback(button_callback);
    input_set_timeout_callback(button_timeout_callback);

    xEventGroup = xEventGroupCreate();

    xTaskCreate(
                vTask1,       /* Function that implements the task. */
                "vTask1",          /* Text name for the task. */
                2048*2,      /* Stack size in words, not bytes. */
                NULL,    /* Parameter passed into the task. */
                4,/* Priority at which the task is created. */
                NULL );      /* Used to pass out the created task's handle. */

}
