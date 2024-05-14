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
#include "utils.h"

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

static void start_leakage_detection(adc_measure_result *measure) {
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
	nvs_close(nvs_h);
	return (int)registerd;
}

static void device_get_config(device_cfg *cfg) {
	nvs_handle_t nvs_h;
	ESP_ERROR_CHECK(nvs_open("cfg", NVS_READONLY, &nvs_h));
	size_t id_l = sizeof(cfg->id);
	size_t lon_l = sizeof(cfg->longitude);
	size_t lat_l = sizeof(cfg->latitude);
	ESP_ERROR_CHECK(nvs_get_str(nvs_h, "id", cfg->id, &id_l));
	ESP_ERROR_CHECK(nvs_get_str(nvs_h, "lon", cfg->longitude, &lon_l));
	ESP_ERROR_CHECK(nvs_get_str(nvs_h, "lat", cfg->latitude, &lat_l));
	nvs_close(nvs_h);
}

static void device_set_config(const device_cfg *cfg) {
	nvs_handle_t nvs_h;
	ESP_ERROR_CHECK(nvs_open("cfg", NVS_READWRITE, &nvs_h));
	ESP_ERROR_CHECK(nvs_set_str(nvs_h, "id", cfg->id));
	ESP_ERROR_CHECK(nvs_set_str(nvs_h, "lon", cfg->longitude));
	ESP_ERROR_CHECK(nvs_set_str(nvs_h, "lat", cfg->latitude));
	nvs_close(nvs_h);
}

static void device_set_registered() {
	nvs_handle_t nvs_h;
	ESP_ERROR_CHECK(nvs_open("info", NVS_READWRITE, &nvs_h));
	ESP_ERROR_CHECK(nvs_set_u8(nvs_h, "registered", 1));
	nvs_close(nvs_h);
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
	device_cfg d_cfg;
	ESP_ERROR_CHECK(wifi_setup_station(&d_cfg));
	if (device_is_registered()) {
		device_get_config(&d_cfg);
	}
	else {
		device_set_config(&d_cfg);
		device_set_registered();
	}
	// now mqtt init, and we pass ip info since the gateway will always be the broker
	ESP_ERROR_CHECK(mqtt_setup(&d_cfg.bridge_ip));
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
	// if some error happened, send to bridge (if possible)
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
