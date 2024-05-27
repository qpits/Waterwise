#include "esp_netif.h"
#include "esp_log.h"
#include <string.h>
#include "mqtt.h"

static esp_mqtt_client_handle_t client;
static const char* TAG = "MQTT";

esp_err_t mqtt_setup(const device_cfg *cfg) {
    char proto[8] = "mqtt://";
    char *broker_uri = memcpy(malloc(255*sizeof(char)), proto, (sizeof(proto) - 1) * sizeof(char));
    char *gw_ip_str = esp_ip4addr_ntoa(&cfg->bridge_ip.gw, malloc(64), 64);
    if (!broker_uri || !gw_ip_str)
        return ESP_FAIL;
    strcpy(broker_uri + 7, gw_ip_str);
    // no clean session bit; store session between network connections
    ESP_LOGI(TAG, "Connecting to MQTT broker: %s", broker_uri);
    esp_mqtt_client_config_t mqtt_cfg = {
        .credentials.client_id = cfg->id,
        .broker.address.uri = broker_uri/*"mqtt://raspberrypi.local"*/,
        .session.disable_clean_session = true
    };
    // create client handle
    esp_mqtt_client_handle_t mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    free(gw_ip_str);
    free(broker_uri);
    if (!mqtt_client)
        return ESP_FAIL;
    client = mqtt_client;
    return ESP_OK;
}

void register_mqtt_event_handlers(esp_event_handler_t event_handler_discovery, void *handler_discovery_args,
                        esp_event_handler_t event_handler_cmd, void *handler_cmd_args) {
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, event_handler_discovery, handler_discovery_args));
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, MQTT_EVENT_DATA, event_handler_cmd, handler_cmd_args));
}

void start_mqtt_client() {
    ESP_ERROR_CHECK(esp_mqtt_client_start(client));
}

void stop_mqtt_client() {
    // simply deallocate the client: this call will disconnect if the client is connected
    ESP_ERROR_CHECK(esp_mqtt_client_disconnect(client));
    ESP_ERROR_CHECK(esp_mqtt_client_destroy(client));
}

void send_message(const char *message, const char *id) {
    char topic[10] = "/";
    strncpy(topic + 1, id, 3);
    strncpy(topic + 4, "/data", 6);
    esp_mqtt_client_publish(client, topic, message, 0, 0, 1);
}

void msg_topic_sub(const char *id) {
    char topic[9] = "/";
    strncpy(topic + 1, id, 3);
    strncpy(topic + 4, "/msg", 5);
    esp_mqtt_client_subscribe(client, topic, 1);
}