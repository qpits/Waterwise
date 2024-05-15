#ifndef EVENT_HANDLERS_H
#define EVENT_HANDLERS_H

#include "esp_event_base.h"
#include <stdint.h>
#include "freertos/event_groups.h"
#include "utils.h"
#include "esp_bit_defs.h"

#define BIT_REGISTER_OK BIT0
#define BIT_REGISTER_FAIL BIT1

struct device_discovery_args {
	device_cfg *device_config;
	EventGroupHandle_t event_grp;
};

void mqtt_event_handler_discovery(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

#endif