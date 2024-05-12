#include "esp_netif_types.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "cJSON.h"

#include "event_handlers.h"

void build_discovery_json(char *buff) {
    cJSON *root = NULL;
    cJSON *location = NULL;
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "id", cJSON_CreateString("ABCD"));
    cJSON_AddItemToObject(root, "location", location = cJSON_CreateObject());
    cJSON_AddStringToObject(location, "coord", "test");
    buff = calloc(256, sizeof(char));
    cJSON_PrintPreallocated(root, buff, 256, false);
    cJSON_Delete(root);
}

static char *TAG = "MQTT_EVN_H";

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
void mqtt_event_handler_discovery(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    struct device_discovery_args *args = (struct device_discovery_args *)handler_args;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "New MQTT connection established.");
        if (!event->session_present) {
            ESP_LOGI(TAG, "Starting discovery procedure...");
            //esp_mqtt_client_publish
            // publish first message
            char *msg_str;
            build_discovery_json(msg_str);
            esp_mqtt_client_publish(event->client, "/discovery", msg_str, 0, 1, 0);
            free(msg_str);
        }
        else {
            ESP_LOGI(TAG, "MQTT session already available, do not need to perform discovery procedure.");
            // simply unregister handler and set the completed flag
            esp_mqtt_client_unregister_event(event->client, ESP_EVENT_ANY_ID, mqtt_event_handler_discovery);
            // set done bit
            xEventGroupSetBits(args->event_grp, BIT_REGISTER_OK);
        }
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        // count disconnection/failures
        // after too many failures, unregister and set fail bit
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        // when published, then we can unregister and set completion flag
        ESP_LOGI(TAG, "Discovery procedure completed successfully.");
        esp_mqtt_client_unregister_event(event->client, ESP_EVENT_ANY_ID, mqtt_event_handler_discovery);
        xEventGroupSetBits(args->event_grp, BIT_REGISTER_OK);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}