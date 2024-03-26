#ifndef WIFI_H
#define WIFI_H

#include "esp_err.h"
#include "esp_netif_types.h"
extern esp_err_t wifi_setup_station(esp_netif_ip_info_t *);
extern void wifi_disconnect_station(void);

#endif