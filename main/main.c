/* ESP32 LED Control Web Server

   This example creates a WiFi Access Point and HTTP server
   to control LED connected to GPIO2 (D2) pin via web interface.
*/

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include <esp_http_server.h>
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* ESP32 LED Control Web Server

   This example creates a WiFi Access Point and HTTP server
   to control LED connected to GPIO2 (D2) pin via web interface.
*/

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include <esp_http_server.h>
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_GPIO_PIN GPIO_NUM_2  // D2 pin on ESP32 DEVKIT_C
#define WIFI_SSID "ESP32_LED_Control"
#define WIFI_PASS "12345678"
#define WIFI_CHANNEL 1
#define MAX_STA_CONN 4

static const char *TAG = "led_control";

// LED state
static bool led_state = false;

// HTML页面内容
static const char* html_page = 
"<!DOCTYPE html>"
"<html>"
"<head>"
"<title>ESP32 LED Control</title>"
"<meta name='viewport' content='width=device-width, initial-scale=1'>"
"<style>"
"body { font-family: Arial, sans-serif; text-align: center; margin: 50px; background-color: #f0f0f0; }"
".container { max-width: 400px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }"
"h1 { color: #333; margin-bottom: 30px; }"
".led-status { font-size: 24px; margin: 20px 0; padding: 15px; border-radius: 5px; }"
".led-on { background-color: #4CAF50; color: white; }"
".led-off { background-color: #f44336; color: white; }"
"button { background-color: #008CBA; color: white; border: none; padding: 15px 30px; font-size: 18px; margin: 10px; cursor: pointer; border-radius: 5px; }"
"button:hover { background-color: #007B9A; }"
".btn-on { background-color: #4CAF50; }"
".btn-on:hover { background-color: #45a049; }"
".btn-off { background-color: #f44336; }"
".btn-off:hover { background-color: #da190b; }"
"</style>"
"</head>"
"<body>"
"<div class='container'>"
"<h1>ESP32 LED Control</h1>"
"<div class='led-status %s'>LED Status: %s</div>"
"<div>"
"<button class='btn-on' onclick='controlLED(1)'>Turn ON</button>"
"<button class='btn-off' onclick='controlLED(0)'>Turn OFF</button>"
"</div>"
"</div>"
"<script>"
"function controlLED(state) {"
"  var xhr = new XMLHttpRequest();"
"  xhr.open('GET', '/led?state=' + state, true);"
"  xhr.onreadystatechange = function() {"
"    if (xhr.readyState == 4 && xhr.status == 200) {"
"      location.reload();"
"    }"
"  };"
"  xhr.send();"
"}"
"</script>"
"</body>"
"</html>";

// 初始化LED GPIO
static void init_led_gpio(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LED_GPIO_PIN),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);
    gpio_set_level(LED_GPIO_PIN, 0); // 初始状态为关闭
    led_state = false;
    ESP_LOGI(TAG, "LED GPIO initialized on pin %d", LED_GPIO_PIN);
}

// 设置LED状态
static void set_led_state(bool state)
{
    gpio_set_level(LED_GPIO_PIN, state ? 1 : 0);
    led_state = state;
    ESP_LOGI(TAG, "LED turned %s", state ? "ON" : "OFF");
}

// 主页处理器
static esp_err_t root_get_handler(httpd_req_t *req)
{
    char response[2048];
    snprintf(response, sizeof(response), html_page, 
             led_state ? "led-on" : "led-off",
             led_state ? "ON" : "OFF");
    
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// LED控制处理器
static esp_err_t led_control_handler(httpd_req_t *req)
{
    char query[64];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        char param[8];
        if (httpd_query_key_value(query, "state", param, sizeof(param)) == ESP_OK) {
            int state = atoi(param);
            set_led_state(state == 1);
        }
    }
    
    // 返回简单的成功响应
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// URI处理器定义
static const httpd_uri_t root = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = root_get_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t led_control = {
    .uri       = "/led",
    .method    = HTTP_GET,
    .handler   = led_control_handler,
    .user_ctx  = NULL
};

// 启动Web服务器
static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &led_control);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

// WiFi事件处理器
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "Station joined, AID=%d", event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "Station left, AID=%d", event->aid);
    }
}

// 初始化WiFi AP模式
static void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .channel = WIFI_CHANNEL,
            .password = WIFI_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    
    if (strlen(WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi AP started. SSID:%s password:%s channel:%d",
             WIFI_SSID, WIFI_PASS, WIFI_CHANNEL);
}

void app_main(void)
{
    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 初始化LED
    init_led_gpio();

    // 初始化WiFi AP
    wifi_init_softap();

    // 启动Web服务器
    httpd_handle_t server = start_webserver();
    if (server) {
        ESP_LOGI(TAG, "Web server started successfully");
        ESP_LOGI(TAG, "Connect to WiFi: %s", WIFI_SSID);
        ESP_LOGI(TAG, "Password: %s", WIFI_PASS);
        ESP_LOGI(TAG, "Open browser and go to: http://192.168.4.1");
    }

    // 主循环
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
