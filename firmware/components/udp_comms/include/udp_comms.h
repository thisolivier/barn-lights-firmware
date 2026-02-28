#pragma once

#include <stddef.h>
#include <stdint.h>

#ifndef UNIT_TEST
#include "esp_err.h"
#else
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_INVALID_ARG -2
#endif

/**
 * Called by the config rx task when a UDP packet arrives.
 * Runs in the rx task context — keep processing short or copy and queue.
 */
typedef void (*udp_config_handler_t)(const uint8_t *data, size_t length);

/**
 * Called by the telemetry tx task to fill the outbound buffer.
 * Returns the number of bytes written into buf (up to max_length).
 */
typedef size_t (*udp_telemetry_packer_t)(uint8_t *buffer, size_t max_length);

typedef struct {
    /* Config channel (laptop -> ESP32) */
    uint16_t config_listen_port;
    udp_config_handler_t config_handler;

    /* Telemetry channel (ESP32 -> laptop) */
    char telemetry_dest_ip[16];
    uint16_t telemetry_dest_port;
    uint32_t telemetry_interval_ms;
    udp_telemetry_packer_t telemetry_packer;

    /* WiFi */
    const char *wifi_ssid;
    const char *wifi_password;
} udp_comms_config_t;

/**
 * Validate config and store it. Does not start tasks or WiFi.
 */
esp_err_t udp_comms_init(const udp_comms_config_t *config);

/**
 * Connect WiFi, then start the config rx and telemetry tx tasks.
 * Blocks until WiFi is connected (or fails).
 */
esp_err_t udp_comms_start(void);

/**
 * Stop tasks and tear down sockets. WiFi stays connected.
 */
void udp_comms_stop(void);
