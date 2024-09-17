#include <stdio.h>
#include <string.h>  
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_system.h"
#include "driver/gpio.h"

#define BLINK_GPIO 2
#define EX_UART_NUM UART_NUM_0
#define PATTERN_CHR_NUM (3)
#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

static QueueHandle_t uart0_queue;
static const char* TAG = "UART";

static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    size_t buffered_size;
    uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);
    for (;;) {
        if (xQueueReceive(uart0_queue, (void *)&event, (TickType_t)portMAX_DELAY)) {
            memset(dtmp, 0, RD_BUF_SIZE);  // Use memset instead of bzero

            switch (event.type) {
            case UART_DATA:
                uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
                uart_write_bytes(EX_UART_NUM, (const char*) dtmp, event.size);

                // Control LED based on received data
                if (dtmp[0] == '1') {
                    gpio_set_level(BLINK_GPIO, 1);
                    ESP_LOGI(TAG, "LED ON");
                } else if (dtmp[0] == '0') {
                    gpio_set_level(BLINK_GPIO, 0);
                    ESP_LOGI(TAG, "LED OFF");
                }
                break;

            case UART_FIFO_OVF:
                ESP_LOGI(TAG, "hw fifo overflow");
                uart_flush_input(EX_UART_NUM);
                xQueueReset(uart0_queue);
                break;

            case UART_BUFFER_FULL:
                ESP_LOGI(TAG, "ring buffer full");
                uart_flush_input(EX_UART_NUM);
                xQueueReset(uart0_queue);
                break;

            case UART_BREAK:
                ESP_LOGI(TAG, "uart rx break");
                break;

            case UART_PARITY_ERR:
                ESP_LOGI(TAG, "uart parity error");
                break;

            case UART_FRAME_ERR:
                ESP_LOGI(TAG, "uart frame error");
                break;

            case UART_PATTERN_DET:
                uart_get_buffered_data_len(EX_UART_NUM, &buffered_size);
                int pos = uart_pattern_pop_pos(EX_UART_NUM);
                ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                if (pos == -1) {
                    uart_flush_input(EX_UART_NUM);
                } else {
                    uart_read_bytes(EX_UART_NUM, dtmp, pos, 100 / portTICK_PERIOD_MS);
                    uint8_t pat[PATTERN_CHR_NUM + 1];
                    memset(pat, 0, sizeof(pat));
                    uart_read_bytes(EX_UART_NUM, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                    ESP_LOGI(TAG, "read data: %s", dtmp);
                    ESP_LOGI(TAG, "read pat : %s", pat);
                }
                break;

            default:
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

void app_main(void)
{
    // Configure the LED pin
    gpio_reset_pin(BLINK_GPIO);  // Use gpio_reset_pin instead of gpio_pad_select_gpio
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    // Configure UART
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_param_config(EX_UART_NUM, &uart_config);
    uart_set_pin(EX_UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart0_queue, 0);

    // Create a task to handle UART events
    xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 12, NULL);
}
