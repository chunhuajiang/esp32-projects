#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#define TAG  "app_sta"

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        ESP_LOGI(TAG, "Connecting to AP...");
        esp_wifi_connect();
        break;

    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "Connected.");
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        //ESP_LOGI(TAG, "Wifi disconnected, try to connect again...");
        esp_wifi_connect();
        break;

    default:
        break;
    }
    
    return ESP_OK;
}

void app_main(void)
{
    nvs_flash_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_APSTA) );
    wifi_config_t sta_config = {
        .sta = {
            .ssid = CONFIG_APSTA_STA_SSID,
            .password = CONFIG_APSTA_STA_PASSWORD,
            .bssid_set = false
        }
    };
    wifi_config_t ap_config = {
        .ap = {
            .ssid = CONFIG_APSTA_AP_SSID,
            .password = CONFIG_APSTA_AP_PASSWORD,
            .ssid_len = 0,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_PSK
        }
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    esp_err_t tmp =  esp_wifi_set_config(WIFI_IF_AP, &ap_config);
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_connect();

}

