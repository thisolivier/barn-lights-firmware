#include "app_config.h"
#include "udp_comms.h"
#include "protocol.h"
#include "sensor.h"
#include "lighting.h"

#ifndef UNIT_TEST
#include "nvs_flash.h"
#include "esp_log.h"

static const char *LOG_TAG = "main";

void app_main(void)
{
    /* NVS is required by WiFi driver */
    esp_err_t nvs_result = nvs_flash_init();
    if (nvs_result == ESP_ERR_NVS_NO_FREE_PAGES ||
        nvs_result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_result = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nvs_result);

    sensor_init();
    lighting_init();

    udp_comms_config_t comms_config = {
        .config_listen_port = UDP_CONFIG_PORT,
        .config_handler = protocol_handle_config,
        .telemetry_dest_ip = UDP_TELEMETRY_DEST_IP,
        .telemetry_dest_port = UDP_TELEMETRY_PORT,
        .telemetry_interval_ms = 1000 / UDP_TELEMETRY_RATE_HZ,
        .telemetry_packer = protocol_pack_telemetry,
        .wifi_ssid = WIFI_SSID,
        .wifi_password = WIFI_PASSWORD,
    };

    ESP_ERROR_CHECK(udp_comms_init(&comms_config));
    esp_err_t start_result = udp_comms_start();
    if (start_result != ESP_OK) {
        ESP_LOGE(LOG_TAG, "UDP comms failed to start: %s",
                 esp_err_to_name(start_result));
        return;
    }

    sensor_start();
    lighting_start();

    ESP_LOGI(LOG_TAG, "Stair lights firmware running");
}

#endif /* UNIT_TEST */
