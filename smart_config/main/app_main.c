#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event_loop.h"
#include "esp_smartconfig.h"
#include "esp_system.h"
#include "nvs_flash.h"

#define     TAG             "app_main"
#define     LINKED_BIT      (BIT0)
#define     CONNECTED_BIT   (BIT1)

static EventGroupHandle_t smartconfig_event_group = NULL;


esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        xEventGroupWaitBits(smartconfig_event_group, LINKED_BIT, false, true, portMAX_DELAY);
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(smartconfig_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

void smartconfig_handler(smartconfig_status_t status, void *pdata)
{
	switch (status) {
	case SC_STATUS_WAIT:
		ESP_LOGI(TAG, "SC_STATUS_WAIT");
		break;
	case SC_STATUS_FIND_CHANNEL:
		ESP_LOGI(TAG, "SC_STATUS_FIND_CHANNEL");
		break;
	case SC_STATUS_GETTING_SSID_PSWD:
		ESP_LOGI(TAG, "SC_STATUS_GETTING_SSID_PSWD");
		smartconfig_type_t *type = pdata;
		if (*type == SC_TYPE_ESPTOUCH) {
			ESP_LOGI(TAG, "SC_TYPE:SC_TYPE_ESPTOUCH");
		} else {
			ESP_LOGI(TAG, "SC_TYPE:SC_TYPE_AIRKISS");
		}
		break;
	case SC_STATUS_LINK:
		ESP_LOGI(TAG, "SC_STATUS_LINK");
		wifi_config_t wifi_config;
		wifi_config.sta = *((wifi_sta_config_t *)pdata);
		esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

		esp_wifi_disconnect();
        xEventGroupSetBits(smartconfig_event_group, LINKED_BIT);
		//esp_wifi_connect();
		break;
	case SC_STATUS_LINK_OVER:
		ESP_LOGI(TAG, "SC_STATUS_LINK_OVER");
		if (pdata != NULL) {
			uint8_t phone_ip[4] = { 0 };

			memcpy(phone_ip, (uint8_t *) pdata, 4);
			ESP_LOGI(TAG, "Phone ip: %d.%d.%d.%d", phone_ip[0],
			       phone_ip[1], phone_ip[2], phone_ip[3]);
		}
		esp_smartconfig_stop();
        xEventGroupSetBits(smartconfig_event_group, CONNECTED_BIT);
		break;
	}

}


void init_wifi(void)
{
	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main()
{
    smartconfig_event_group = xEventGroupCreate();
	
	init_wifi();

    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS));
	ESP_ERROR_CHECK(esp_smartconfig_start(smartconfig_handler));
    
    xEventGroupWaitBits(smartconfig_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);

    ESP_LOGI(TAG, "Smart Config Done!")
}
