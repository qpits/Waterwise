#ifndef MQTT_H
#define MQTT_H

#include "esp_netif_types.h"
#include "mqtt_client.h"
#include "utils.h"

esp_err_t mqtt_setup(const device_cfg *cfg);
void register_mqtt_event(esp_event_handler_t event_handler, void *handler_args);
void start_mqtt_client();
void stop_mqtt_client();
void send_message(const char *message, const char *id);
#endif