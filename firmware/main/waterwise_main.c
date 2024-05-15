#include "esp_event.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_check.h"
#include "freertos/event_groups.h"
#include "esp_sleep.h"

#include <math.h>

#include "wifi.h"
#include "mqtt.h"
#include "adc.h"
#include "event_handlers.h"
#include "utils.h"
#include "inference.h"

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
	// TODO: set timer wakeup interval in configuration
	esp_sleep_enable_timer_wakeup(10000000);
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
		ESP_LOGI(TAG, "Device is already registered: read from flash...");
		device_get_config(&d_cfg);
	}
	else {
		ESP_LOGI(TAG, "Device has been configured. Saving config to flash...");
		device_set_config(&d_cfg);
		device_set_registered();
	}
	// now mqtt init, and we pass ip info since the gateway will always be the broker
	ESP_ERROR_CHECK(mqtt_setup(&d_cfg));
	struct device_discovery_args arg = {
		.device_config = &d_cfg,
		.event_grp = task_events
	};
	register_mqtt_event(mqtt_event_handler_discovery, &arg);
	start_mqtt_client();
	// wait for bits to be set
	ESP_LOGI(TAG, "Waiting for registration...");
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
	esp_err_t err = adc_reading(1000, &measure);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Error during measure.");
		goto clean;
	}
	// store inference result
	uint16_t *subsampled = subsample_measure(&measure);
	float inference_res[2];
	ESP_ERROR_CHECK(start_inference((int16_t *)subsampled, inference_res));
	ESP_LOGI(TAG, "Inference result: %f, %f", inference_res[0], inference_res[1]);
	char *result_msg = NULL;
	build_inference_res_str(&result_msg, inference_res);
	send_message(result_msg, d_cfg.id);
	free(result_msg);
	free(subsampled);
	free(measure.measure_buff);
	clean:
	// stop mqtt connection
	stop_mqtt_client();
	/* disconnect and turn off wifi. control flow will not arrive here if wifi is not initialized */
	wifi_disconnect_station();
	// go to sleep
	esp_deep_sleep_start();
}
