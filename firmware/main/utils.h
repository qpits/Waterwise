#ifndef UTILS_H
#define UTILS_H

#include "esp_netif_types.h"
#include "adc.h"
#define BLINK_GPIO GPIO_NUM_14

typedef struct device_cfg {
    esp_netif_ip_info_t bridge_ip;
    char latitude[11];
    char longitude[11];
    char id[4];
} device_cfg;

typedef enum command_id {
    SIGNAL_LEAK,
    SIGNAL_OK
} command_id;

typedef struct command {
    command_id id;
} command;

int parse_device_data_str(device_cfg *cfg, const char *data);
uint16_t *subsample_measure(adc_measure_result *res);
void build_inference_res_str(char **buff, float *res);
command *parse_message_str(char *msg_str);
void configure_led(void);
void blink_led(uint8_t level);
#endif