#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "cJSON.h"

int parse_device_data_str(device_cfg *cfg, const char *data) {
    char id[4];
    char longitude[11];
    char latitude[11];

    int res = sscanf(data, "%3s#%10s,%10s", id, latitude, longitude);
    if (res == 3) {
        memmove(&cfg->id, id, sizeof(id));
        memmove(&cfg->longitude, longitude, sizeof(longitude));
        memmove(&cfg->latitude, latitude, sizeof(latitude));
        return true;
    }
    else return false;
}

uint16_t *subsample_measure(adc_measure_result *res) {
    // should always downsample to 2500
    // compute stride
    div_t q = div(res->samples, 2500);
    uint32_t stride = q.quot;
    uint32_t window = 2*stride - 1;
    uint16_t *out = malloc(2500 * sizeof(uint16_t));
    size_t n = 0;
    for (size_t i = 0; i < res->samples; i+=stride) {
        long long k = i - stride + 1;
        int j = 0;
        float mean = 0.;
        while (k + j < 0) {
            mean += res->measure_buff[0];
            j++;
        }
        for (; j < window && k + j < res->samples; j++)
            mean += res->measure_buff[k+j];

        while (k + j >= res->samples && j < window) {
            mean += res->measure_buff[res->samples];
            j++;
        }
        out[n++] = (uint16_t)round(mean/window);
    }
    assert(n == 2500);
    return out;
}

void build_inference_res_str(char **buff, float *res) {
    *buff = calloc(128, sizeof(char));
    cJSON *root = cJSON_CreateObject();
    cJSON *result = NULL;
    cJSON_AddItemToObject(root, "result", result = cJSON_CreateObject());
    cJSON_AddNumberToObject(result, "0", res[0]);
    cJSON_AddNumberToObject(result, "1", res[1]);
    cJSON_PrintPreallocated(root, *buff, 128, false);
    cJSON_Delete(root);
}

command *parse_message_str(char *msg_str) {
    cJSON *msg = cJSON_Parse(msg_str);
    if (!msg)
        return NULL;
    cJSON *id = cJSON_GetObjectItem(msg, "id");
    if(!cJSON_IsNumber(id))
        return NULL;
    command *cmd = malloc(sizeof(command));
    cmd->id = (enum command_id)id->valueint;
    cJSON_Delete(msg);
    return cmd;
}