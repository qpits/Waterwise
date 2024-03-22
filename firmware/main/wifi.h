#ifndef WIFI_H
#define WIFI_H

#include "esp_err.h"
extern esp_err_t wifi_setup_station(void);
extern void wifi_disconnect_station(void);

#endif