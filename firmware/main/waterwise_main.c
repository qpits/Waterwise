#include "esp_event.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "wifi.h"

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // create default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    /* setup wifi */
    // in case we fail, abort and restart application.
    ESP_ERROR_CHECK(wifi_setup_station());
    for(;;) {
        vTaskDelay(1000);
    }
    /* disconnect and turn off wifi. control flow will not arrive here if wifi is not initialized */
    wifi_disconnect_station();
}
