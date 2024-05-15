#include "inference.h"
#include "model.h"
#include <array>
#include <cstring>
#include "esp_err.h"

EXTERNC esp_err_t start_inference(int16_t *samples, float *result) {
    if (!setup_model())
        return ESP_FAIL;
    std::array<float, 2> res = predict(samples);
    memmove(result, res.data(), 2*sizeof(float));
    return ESP_OK;
}