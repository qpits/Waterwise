#include "esp_event.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_check.h"
#include "freertos/event_groups.h"

#include <math.h>

#include "wifi.h"
#include "mqtt.h"
#include "adc.h"
#include "event_handlers.h"

static const char *TAG = "MAIN";

static esp_mqtt_client_handle_t mqtt_client;
static EventGroupHandle_t task_events;

void setup_app_event_loop(esp_event_loop_handle_t *loop) {
	esp_event_loop_args_t app_events_args = {
		.queue_size = 5,
		.task_name = NULL
	};
	ESP_ERROR_CHECK(esp_event_loop_create(&app_events_args, loop));
}

void report_measure_error(esp_err_t err) {
	esp_mqtt_client_enqueue(mqtt_client, "/test/error", "Error occurred during measure", 0, 1, 0, false);
}

static void leak_detect_task(adc_measure_result *measure) {
	float mean = 0.0f;
	float variance = 0.0f;
	for (size_t i = 0; i < measure->samples; i++) {
		mean += measure->measure_buff[i];
	}
	mean /= (float)measure->samples;

	for (size_t i = 0; i < measure->samples; i++) {
		variance += powf(measure->measure_buff[i] - mean, 2.0f);
	}
	variance /= ((float) measure->samples - 1);

	if (variance > SOME_MAGIC_NUMBER)
		// set bits and maybe set result.

}

static void start_leakage_detection(adc_measure_result *measure) {
	// start task for leakage detection
	// this task will probably send an event or more simply set an event bit to signal when it finished
	// then another function will collect the results.


}

static int device_is_registered() {
	nvs_handle_t nvs_h;
	ESP_ERROR_CHECK(nvs_open("info", NVS_READWRITE, &nvs_h));
	// check "registered" flag to know if this device is registered to the bridge
	uint8_t registerd;
	esp_err_t err = nvs_get_u8(nvs_h, "registered", &registerd);
	if (err == ESP_ERR_NVS_NOT_FOUND) {
		registerd = 0;
	}
	else if (err != ESP_OK) {
		ESP_ERROR_CHECK(err);
	}
	return (int)registerd;
}

void app_main(void)
{
	StaticEventGroup_t event_group_buff;
	task_events = xEventGroupCreateStatic(&event_group_buff);
	if (task_events == NULL) {
		ESP_LOGE(TAG, "Failed to initialize event group.");
		abort();
	}

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
	ESP_ERROR_CHECK(mqtt_setup(&ip_info_handle));
	struct device_discovery_args arg = {
		.failed_register_count = 5,
		.event_grp = task_events
	};
	register_mqtt_event(mqtt_event_handler_discovery, &arg);
	start_mqtt_client();
	// wait for bits to be set
	EventBits_t bits = xEventGroupWaitBits(task_events,
			BIT_REGISTER_OK | BIT_REGISTER_FAIL, 
			pdTRUE, 
			pdFALSE, 
			portMAX_DELAY);
	if (bits & BIT_REGISTER_FAIL) {
		// failed to register: clean up and go back to sleep. will try again later.
		ESP_LOGE(TAG, "Failed to register with bridge.");
		goto clean;
	}
	else if (!(bits & BIT_REGISTER_OK)){
		ESP_LOGE(TAG, "Unexpected event during registration procedure.");
		abort();
	}
	// read from adc
	adc_measure_result measure;
	esp_err_t err = adc_reading(2000, &measure);
	// if some error happened, send error if possible to bridge
	err = ESP_FAIL;
	if (err != ESP_OK) {
		report_measure_error(err);
		goto clean;
	}
	for(;;) {
		vTaskDelay(1000);
	}
	clean:
	// stop mqtt connection
	stop_mqtt_client();
	/* disconnect and turn off wifi. control flow will not arrive here if wifi is not initialized */
	wifi_disconnect_station();
	// go to sleep
}
