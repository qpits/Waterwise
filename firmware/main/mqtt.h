#ifndef MQTT_H
#define MQTT_H

#include "esp_netif_types.h"
#include "mqtt_client.h"

esp_err_t mqtt_setup(const esp_netif_ip_info_t *ip);
void register_mqtt_event(esp_event_handler_t event_handler, void *handler_args);
void start_mqtt_client();
void stop_mqtt_client();
#endif