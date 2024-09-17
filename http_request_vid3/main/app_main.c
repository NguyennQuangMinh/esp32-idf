#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "api.thingspeak.com"
#define WEB_PORT "80"
#define WEB_PATH "/update"

static const char *TAG = "example";
char REQUEST[512];
char SUBREQUEST[100];
char recv_buf[512];

static void http_get_task(void *pvParameters)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;

    while (1) {
        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);

        if (err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if (s < 0) {
            ESP_LOGE(TAG, "Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "Allocated socket");

        if (connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "Socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "Connected");
        freeaddrinfo(res);

        // Build the POST request
        //sprintf(SUBREQUEST, "api_key=8J05LSJCXW79CB4N&field1=%d&field2=%d", 25, 25);
        // sprintf(REQUEST, "POST " WEB_PATH " HTTP/1.1\r\n"
        //                  "Host: " WEB_SERVER "\r\n"
        //                  "Connection: close\r\n"
        //                  "Content-Type: application/x-www-form-urlencoded\r\n"
        //                  "Content-Length: %d\r\n\r\n"
        //                  "%s\r\n", strlen(SUBREQUEST), SUBREQUEST);
        //sprintf(REQUEST, "GET http://api.thingspeak.com/update.json?api_key=8J05LSJCXW79CB4N&field1=80&field2=80\n\n");
        sprintf(REQUEST, "GET http://api.thingspeak.com/channels/2602329/feeds.json?api_key=8865O6MJP6C68QHY&results=2\n\n");

        if (write(s, REQUEST, strlen(REQUEST)) < 0) {
            ESP_LOGE(TAG, "Socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "Socket send success");

        // Set socket receiving timeout
        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout, sizeof(receiving_timeout)) < 0) {
            ESP_LOGE(TAG, "Failed to set socket receiving timeout");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "Set socket receiving timeout success");

        // Read HTTP response
        do {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf) - 1);
            if (r > 0) {
                for (int i = 0; i < r; i++) {
                    putchar(recv_buf[i]);
                }
            }
        } while (r > 0);

        ESP_LOGI(TAG, "Done reading from socket. Last read return=%d errno=%d.", r, errno);
        close(s);

        // Delay before starting again
        for (int countdown = 10; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d... ", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Starting again!");
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
    ESP_ERROR_CHECK(example_connect());

    xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
}
