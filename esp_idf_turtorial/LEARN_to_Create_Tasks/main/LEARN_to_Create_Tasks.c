#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

#define LED_PIN 2
TaskHandle_t myTaskHandle = NULL;
TaskHandle_t myTaskHandle2 = NULL;

void Demo_Task(void *arg)
{
    int count = 0;
    while(1){
        count++;
        printf("Demo_Task printing..\n");
        vTaskDelay(1000/ portTICK_PERIOD_MS);
        if( count == 5 )
        {
            vTaskSuspend(myTaskHandle2);
            printf("Demo_Task 2 is suspend!!!\n");
        }
        if(count == 10)
        {
            vTaskResume(myTaskHandle2);
            printf("Demo_Task 2 is resume!!!\n");
        }
    }
}

void Demo_Task2(void *arg)
{
    esp_rom_gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    int ON = 0;
    while(1)
    {
        ON = !ON;
        gpio_set_level(LED_PIN, ON);
        vTaskDelay(1000/ portTICK_PERIOD_MS);
        printf("Turning the LED %d!\n", ON);
    }
}

void app_main(void)
{
   xTaskCreate(Demo_Task, "Demo_Task", 4096, NULL, 10, &myTaskHandle);
   xTaskCreatePinnedToCore(Demo_Task2, "Demo_Task2", 4096, NULL,10, &myTaskHandle2, 1);
 }