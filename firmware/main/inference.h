#ifndef INFERENCE_H
#define INFERENCE_H

#include "esp_err.h"
#ifdef __cplusplus
#include <cstdint>
#define EXTERNC extern "C"
extern "C" {
#else
#include <stdint.h>
#endif

esp_err_t start_inference(int16_t *samples, float *result);

#ifdef __cplusplus
}
#endif

#endif