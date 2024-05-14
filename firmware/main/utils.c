#include "utils.h"
#include <stdio.h>
#include <string.h>

int parse_device_data_str(device_cfg *cfg, const char *data) {
    char id[4];
    char longitude[10];
    char latitude[10];

    int res = sscanf(data, "%3s#%9s,%9s", id, latitude, longitude);
    if (res == 3) {
        memmove(&cfg->id, id, sizeof(id));
        memmove(&cfg->longitude, longitude, sizeof(longitude));
        memmove(&cfg->latitude, latitude, sizeof(latitude));
        return true;
    }
    else return false;
}