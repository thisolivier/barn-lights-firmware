#include "unity.h"
#include "udp_comms.h"
#include <string.h>

/* Expose test helper from udp_comms.c UNIT_TEST build */
extern const udp_comms_config_t *udp_comms_get_config(void);

static void dummy_config_handler(const uint8_t *data, size_t length)
{
    (void)data;
    (void)length;
}

static size_t dummy_telemetry_packer(uint8_t *buffer, size_t max_length)
{
    (void)buffer;
    (void)max_length;
    return 0;
}

static udp_comms_config_t make_valid_config(void)
{
    udp_comms_config_t config;
    memset(&config, 0, sizeof(config));
    config.config_listen_port = 49701;
    config.config_handler = dummy_config_handler;
    strncpy(config.telemetry_dest_ip, "192.168.1.100",
            sizeof(config.telemetry_dest_ip));
    config.telemetry_dest_port = 49700;
    config.telemetry_interval_ms = 100;
    config.telemetry_packer = dummy_telemetry_packer;
    config.wifi_ssid = "TestSSID";
    config.wifi_password = "TestPass";
    return config;
}

void setUp(void) {}
void tearDown(void) {}

void test_init_succeeds_with_valid_config(void)
{
    udp_comms_config_t config = make_valid_config();
    esp_err_t result = udp_comms_init(&config);
    TEST_ASSERT_EQUAL(ESP_OK, result);
}

void test_init_rejects_null_config(void)
{
    esp_err_t result = udp_comms_init(NULL);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, result);
}

void test_init_rejects_null_config_handler(void)
{
    udp_comms_config_t config = make_valid_config();
    config.config_handler = NULL;
    esp_err_t result = udp_comms_init(&config);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, result);
}

void test_init_rejects_null_telemetry_packer(void)
{
    udp_comms_config_t config = make_valid_config();
    config.telemetry_packer = NULL;
    esp_err_t result = udp_comms_init(&config);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, result);
}

void test_init_rejects_null_wifi_ssid(void)
{
    udp_comms_config_t config = make_valid_config();
    config.wifi_ssid = NULL;
    esp_err_t result = udp_comms_init(&config);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, result);
}

void test_init_rejects_null_wifi_password(void)
{
    udp_comms_config_t config = make_valid_config();
    config.wifi_password = NULL;
    esp_err_t result = udp_comms_init(&config);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, result);
}

void test_init_rejects_zero_telemetry_interval(void)
{
    udp_comms_config_t config = make_valid_config();
    config.telemetry_interval_ms = 0;
    esp_err_t result = udp_comms_init(&config);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, result);
}

void test_init_stores_config_correctly(void)
{
    udp_comms_config_t config = make_valid_config();
    udp_comms_init(&config);

    const udp_comms_config_t *stored = udp_comms_get_config();
    TEST_ASSERT_EQUAL(49701, stored->config_listen_port);
    TEST_ASSERT_EQUAL(49700, stored->telemetry_dest_port);
    TEST_ASSERT_EQUAL(100, stored->telemetry_interval_ms);
    TEST_ASSERT_EQUAL_STRING("192.168.1.100", stored->telemetry_dest_ip);
    TEST_ASSERT_EQUAL_PTR(dummy_config_handler, stored->config_handler);
    TEST_ASSERT_EQUAL_PTR(dummy_telemetry_packer, stored->telemetry_packer);
}

void test_start_returns_ok(void)
{
    udp_comms_config_t config = make_valid_config();
    udp_comms_init(&config);
    esp_err_t result = udp_comms_start();
    TEST_ASSERT_EQUAL(ESP_OK, result);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_init_succeeds_with_valid_config);
    RUN_TEST(test_init_rejects_null_config);
    RUN_TEST(test_init_rejects_null_config_handler);
    RUN_TEST(test_init_rejects_null_telemetry_packer);
    RUN_TEST(test_init_rejects_null_wifi_ssid);
    RUN_TEST(test_init_rejects_null_wifi_password);
    RUN_TEST(test_init_rejects_zero_telemetry_interval);
    RUN_TEST(test_init_stores_config_correctly);
    RUN_TEST(test_start_returns_ok);
    return UNITY_END();
}
