#include <string.h>
#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "esp_event_base.h"
#include "esp_log.h"

#include "esp_smartconfig.h"

#include "wifi.h"

#define MAX_RETRY 5
#define WIFI_FAIL_BIT BIT0
#define WIFI_CONNECTED_BIT BIT1
#define ESPTOUCH_DONE_BIT BIT2
static unsigned int s_retry_num;
static EventGroupHandle_t s_wifi_event_group;
static const char *TAG = "WIFI";
/* SmartConfig(TM) functionality implemented following Espressif tutorial: https://github.com/espressif/esp-idf/tree/v5.2.1/examples/wifi/smart_config*/

static void smartconfig_start() {
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    smartconfig_start_config_t sc_cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&sc_cfg));
}

/* this function will be called to handle events posted on the default loop by the wifi driver */
static void connection_event_handler(void *args, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        // event has been relayed to the app task. recommended to call connect here.
        // we can do this also after the event arises
        // check for config into NVS flash
        wifi_config_t wifi_cfg;
        ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &wifi_cfg));
        // To check if a valid configuration was read, we need to check if the length of the ssid is valid (> 0); otherwise start provisioning
        if (wifi_cfg.sta.ssid[0]) {
            ESP_LOGI(TAG, "Loading wifi configuration from NVS.");
            esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg);
            esp_wifi_connect();
        }
        else {
            ESP_LOGI(TAG, "Wifi config not found in NVS. Starting SmartConfig service.");
            smartconfig_start();
        }
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Connection lost: retry number %u out of %d.", s_retry_num, MAX_RETRY);
        }
        else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(TAG, "Got SSID and password from SC.");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));

        ESP_LOGI(TAG, "SSID:%s", evt->ssid);
        // config received, now try to connect with the specified AP
        ESP_ERROR_CHECK( esp_wifi_disconnect() );
        ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
        esp_wifi_connect();
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
}

esp_err_t wifi_setup_station(void)
{
    esp_err_t err = ESP_FAIL;
    /* NOTE: default event loop will be created in main task before calling this function */
    s_wifi_event_group = xEventGroupCreate();
    // initialize Lightweight TCP/IP stack (LwIP) and launch its core task
    ESP_ERROR_CHECK(esp_netif_init());

    // Now we need to create the default netwoerk interface instance binding between wifi (AP or station) and the TCP stack
    // in case of errors, this call aborts the program.
    esp_netif_t *netif_handle = esp_netif_create_default_wifi_sta();
    assert(netif_handle);
    (void)netif_handle; // avoid call optimization.

    // create configuration for wifi driver init, then call initializer
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        connection_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        connection_event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    ESP_ERROR_CHECK( esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, connection_event_handler, NULL) );
    // CONFIG WILL BE DONE WITH SMARTCONFIG
    // all configuration will be stored to flash and persist between boots
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    // USE SMARTCONFIG ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg), TAG, "Failed to set wifi configuration.");
    ESP_ERROR_CHECK(esp_wifi_start());
    // if we cannot connect, return fail to caller
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to AP");
        err = ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect to SSID.");
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
    /* Unregister event handler. In case of disconnection after this point there will not be a reconnection retry until the app is restarted */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, connection_event_handler));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, connection_event_handler));
    return err;
}

void wifi_disconnect_station(void) {
    /* disconncet station
        if the wifi was successfully connected, then these calls will hopefully not fail.
    */
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());
}