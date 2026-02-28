#include "udp_comms.h"

#ifndef UNIT_TEST

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "lwip/sockets.h"

#include <string.h>

static const char *LOG_TAG = "udp_comms";

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define MAX_WIFI_RETRIES   10

#define CONFIG_RX_BUF_SIZE 256
#define TELEMETRY_TX_BUF_SIZE 256

static udp_comms_config_t stored_config;
static EventGroupHandle_t wifi_event_group;
static int wifi_retry_count;
static volatile bool tasks_running;

static TaskHandle_t config_rx_task_handle;
static TaskHandle_t telemetry_tx_task_handle;

/* ---- WiFi event handlers ---- */

static void wifi_event_handler(void *argument, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (wifi_retry_count < MAX_WIFI_RETRIES) {
            esp_wifi_connect();
            wifi_retry_count++;
            ESP_LOGI(LOG_TAG, "Retrying WiFi connection (%d/%d)",
                     wifi_retry_count, MAX_WIFI_RETRIES);
        } else {
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGE(LOG_TAG, "WiFi connection failed after %d retries",
                     MAX_WIFI_RETRIES);
        }
    }
}

static void ip_event_handler(void *argument, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(LOG_TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        wifi_retry_count = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static esp_err_t wifi_init_sta(const char *ssid, const char *password)
{
    wifi_event_group = xEventGroupCreate();
    wifi_retry_count = 0;

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

    esp_event_handler_instance_t wifi_handler_instance;
    esp_event_handler_instance_t ip_handler_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID,
        &wifi_event_handler, NULL, &wifi_handler_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP,
        &ip_event_handler, NULL, &ip_handler_instance));

    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, password,
            sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(LOG_TAG, "Connecting to WiFi SSID: %s", ssid);

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(LOG_TAG, "WiFi connected");
        return ESP_OK;
    }
    ESP_LOGE(LOG_TAG, "WiFi connection failed");
    return ESP_FAIL;
}

/* ---- Config RX task ---- */

static void config_rx_task(void *parameter)
{
    int socket_descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_descriptor < 0) {
        ESP_LOGE(LOG_TAG, "Config rx socket creation failed: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    struct sockaddr_in bind_address = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(stored_config.config_listen_port),
    };

    int bind_result = bind(socket_descriptor,
                           (struct sockaddr *)&bind_address,
                           sizeof(bind_address));
    if (bind_result < 0) {
        ESP_LOGE(LOG_TAG, "Config rx bind failed: errno %d", errno);
        close(socket_descriptor);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(LOG_TAG, "Config rx listening on port %d",
             stored_config.config_listen_port);

    uint8_t receive_buffer[CONFIG_RX_BUF_SIZE];

    while (tasks_running) {
        ssize_t received_length = recvfrom(socket_descriptor, receive_buffer,
                                           sizeof(receive_buffer), 0, NULL, NULL);
        if (received_length > 0 && stored_config.config_handler != NULL) {
            stored_config.config_handler(receive_buffer, (size_t)received_length);
        }
    }

    close(socket_descriptor);
    vTaskDelete(NULL);
}

/* ---- Telemetry TX task ---- */

static void telemetry_tx_task(void *parameter)
{
    int socket_descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_descriptor < 0) {
        ESP_LOGE(LOG_TAG, "Telemetry tx socket creation failed: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    /* Enable broadcast if destination is a broadcast address */
    int broadcast_enable = 1;
    setsockopt(socket_descriptor, SOL_SOCKET, SO_BROADCAST,
               &broadcast_enable, sizeof(broadcast_enable));

    struct sockaddr_in dest_address = {
        .sin_family = AF_INET,
        .sin_port = htons(stored_config.telemetry_dest_port),
    };
    inet_pton(AF_INET, stored_config.telemetry_dest_ip,
              &dest_address.sin_addr);

    ESP_LOGI(LOG_TAG, "Telemetry tx sending to %s:%d every %lu ms",
             stored_config.telemetry_dest_ip,
             stored_config.telemetry_dest_port,
             (unsigned long)stored_config.telemetry_interval_ms);

    uint8_t transmit_buffer[TELEMETRY_TX_BUF_SIZE];

    while (tasks_running) {
        if (stored_config.telemetry_packer != NULL) {
            size_t packed_length = stored_config.telemetry_packer(
                transmit_buffer, sizeof(transmit_buffer));
            if (packed_length > 0) {
                ssize_t sent_length = sendto(
                    socket_descriptor, transmit_buffer, packed_length, 0,
                    (struct sockaddr *)&dest_address, sizeof(dest_address));
                if (sent_length < 0) {
                    ESP_LOGW(LOG_TAG, "Telemetry sendto failed: errno %d", errno);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(stored_config.telemetry_interval_ms));
    }

    close(socket_descriptor);
    vTaskDelete(NULL);
}

/* ---- Public API ---- */

esp_err_t udp_comms_init(const udp_comms_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (config->config_handler == NULL || config->telemetry_packer == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (config->wifi_ssid == NULL || config->wifi_password == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (config->telemetry_interval_ms == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(&stored_config, config, sizeof(udp_comms_config_t));
    tasks_running = false;
    return ESP_OK;
}

esp_err_t udp_comms_start(void)
{
    esp_err_t wifi_result = wifi_init_sta(stored_config.wifi_ssid,
                                          stored_config.wifi_password);
    if (wifi_result != ESP_OK) {
        return wifi_result;
    }

    tasks_running = true;

    xTaskCreate(config_rx_task, "config_rx", 4096, NULL, 5,
                &config_rx_task_handle);
    xTaskCreate(telemetry_tx_task, "telemetry_tx", 4096, NULL, 5,
                &telemetry_tx_task_handle);

    return ESP_OK;
}

void udp_comms_stop(void)
{
    tasks_running = false;
    /* Tasks will exit their loops and self-delete on next iteration.
       The rx task blocks on recvfrom so it won't stop until the next
       packet arrives or socket is closed. For a clean stop we could
       close the socket from here, but for the MVP this is sufficient. */
}

#else /* UNIT_TEST */

/* Stub implementations for host-side testing */

static udp_comms_config_t stored_config;

esp_err_t udp_comms_init(const udp_comms_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (config->config_handler == NULL || config->telemetry_packer == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (config->wifi_ssid == NULL || config->wifi_password == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (config->telemetry_interval_ms == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    stored_config = *config;
    return ESP_OK;
}

esp_err_t udp_comms_start(void)
{
    return ESP_OK;
}

void udp_comms_stop(void)
{
}

/* Expose stored config for tests to invoke callbacks directly */
const udp_comms_config_t *udp_comms_get_config(void)
{
    return &stored_config;
}

#endif /* UNIT_TEST */
