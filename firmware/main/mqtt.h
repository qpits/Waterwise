#ifndef MQTT_H
#define MQTT_H

#include "esp_netif_types.h"
#include "mqtt_client.h"
esp_err_t mqtt_setup(esp_mqtt_client_handle_t *client, const esp_netif_ip_info_t *ip);

#endif