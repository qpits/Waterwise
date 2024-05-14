#ifndef UTILS_H
#define UTILS_H

#include "esp_netif_types.h"
typedef struct device_cfg {
    esp_netif_ip_info_t bridge_ip;
    char latitude[10];
    char longitude[10];
    char id[4];
} device_cfg;

int parse_device_data_str(device_cfg *cfg, const char *data);

#endif