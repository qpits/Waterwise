#ifndef ADC_H
#define ADC_H
#include "esp_err.h"

#include <stdint.h>

typedef struct {
    size_t samples;
    uint16_t *measure_buff;
} adc_measure_result;

extern esp_err_t adc_reading(uint32_t length_ms, adc_measure_result *result);
#endif