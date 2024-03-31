#include "esp_event.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "wifi.h"
#include "mqtt.h"
#include "adc.h"

static esp_mqtt_client_handle_t mqtt_client;

void setup_app_event_loop(esp_event_loop_handle_t *loop) {
	esp_event_loop_args_t app_events_args = {
		.queue_size = 5,
		.task_name = NULL
	};
	ESP_ERROR_CHECK(esp_event_loop_create(&app_events_args, loop));
}

void app_main(void)
{
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// create default event loop
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	/* setup wifi */
	// in case we fail, abort and restart application.
	esp_netif_ip_info_t ip_info_handle;
	ESP_ERROR_CHECK(wifi_setup_station(&ip_info_handle));
	// now mqtt init, and we pass ip info since the gateway will always be the broker
	ESP_ERROR_CHECK(mqtt_setup(&mqtt_client, &ip_info_handle));
	
	// read from adc
	adc_measure_result measure;
	esp_err_t err = adc_reading(2000, &measure);
	// if some error happened, send error if possible to bridge
	for(;;) {
		vTaskDelay(1000);
	}
	/* disconnect and turn off wifi. control flow will not arrive here if wifi is not initialized */
	wifi_disconnect_station();
}
