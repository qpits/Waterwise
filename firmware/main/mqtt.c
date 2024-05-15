#include "esp_netif.h"
#include "esp_log.h"
#include <string.h>
#include "mqtt.h"

static esp_mqtt_client_handle_t client;

esp_err_t mqtt_setup(const device_cfg *cfg) {
    char broker_uri[255] = "mqtt://";
    char *gw_ip_str = esp_ip4addr_ntoa(&cfg->bridge_ip.gw, malloc(64), 64);
    assert(gw_ip_str);
    memcpy(broker_uri + 7, gw_ip_str, strlen(gw_ip_str));
    // no clean session bit; store session between network connections
    esp_mqtt_client_config_t mqtt_cfg = {
        .credentials.client_id = cfg->id,
        .broker.address.uri = "mqtt://raspberrypi.local",
        .session.disable_clean_session = true
    };
    // create client handle
    esp_mqtt_client_handle_t mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    free(gw_ip_str);
    if (!mqtt_client)
        return ESP_FAIL;
    client = mqtt_client;
    return ESP_OK;
}

void register_mqtt_event(esp_event_handler_t event_handler, void *handler_args) {
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, event_handler, handler_args));
}

void start_mqtt_client() {
    ESP_ERROR_CHECK(esp_mqtt_client_start(client));
}

void stop_mqtt_client() {
    // simply deallocate the client: this call will disconnect if the client is connected
    ESP_ERROR_CHECK(esp_mqtt_client_destroy(client));
}

void send_message(const char *message, const char *id) {
    char topic[5] = "/";
    strncpy(topic + 1, id, 3);
    esp_mqtt_client_publish(client, topic, message, 0, 0, 0);
}