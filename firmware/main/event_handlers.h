#ifndef EVENT_HANDLERS_H
#define EVENT_HANDLERS_H

#include "esp_event_base.h"
#include <stdint.h>
#include "freertos/event_groups.h"

#define BIT_REGISTER_OK 0
#define BIT_REGISTER_FAIL 1

struct device_discovery_args {
	int failed_register_count;
	EventGroupHandle_t event_grp;
};

void mqtt_event_handler_discovery(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

#endif