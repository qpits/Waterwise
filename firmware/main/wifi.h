#ifndef WIFI_H
#define WIFI_H

#include "esp_err.h"
#include "utils.h"
extern esp_err_t wifi_setup_station(device_cfg *dev_cfg);
extern void wifi_disconnect_station(void);

#endif