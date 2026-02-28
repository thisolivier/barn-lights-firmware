#include "unity.h"
#include "protocol.h"
#include "sensor.h"
#include "lighting.h"
#include "app_config.h"

void setUp(void) {}
void tearDown(void) {}

void test_handle_config_does_not_crash_on_empty_packet(void)
{
    uint8_t empty_data[] = {0};
    protocol_handle_config(empty_data, 0);
    /* Phase 1: just verifies the stub doesn't crash.
       Phase 2 will add real parsing tests. */
}

void test_pack_telemetry_returns_zero_for_stub(void)
{
    uint8_t buffer[256];
    size_t packed = protocol_pack_telemetry(buffer, sizeof(buffer));
    /* Phase 1 stub returns 0 */
    TEST_ASSERT_EQUAL(0, packed);
}

void test_sensor_get_latest_returns_zero_initially(void)
{
    sensor_init();
    for (int step = 0; step < NUM_STAIRS; step++) {
        TEST_ASSERT_EQUAL(0, sensor_get_latest(step));
    }
}

void test_sensor_get_latest_out_of_range_returns_zero(void)
{
    sensor_init();
    TEST_ASSERT_EQUAL(0, sensor_get_latest(NUM_STAIRS));
    TEST_ASSERT_EQUAL(0, sensor_get_latest(255));
}

void test_lighting_set_and_get_step(void)
{
    lighting_init();
    lighting_set_step(0, 4095);
    TEST_ASSERT_EQUAL(4095, lighting_get_step(0));

    lighting_set_step(15, 1000);
    TEST_ASSERT_EQUAL(1000, lighting_get_step(15));
}

void test_lighting_set_all(void)
{
    lighting_init();
    lighting_set_all(2048);
    for (int step = 0; step < NUM_STAIRS; step++) {
        TEST_ASSERT_EQUAL(2048, lighting_get_step(step));
    }
}

void test_lighting_out_of_range_ignored(void)
{
    lighting_init();
    lighting_set_step(NUM_STAIRS, 100);
    TEST_ASSERT_EQUAL(0, lighting_get_step(NUM_STAIRS));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_handle_config_does_not_crash_on_empty_packet);
    RUN_TEST(test_pack_telemetry_returns_zero_for_stub);
    RUN_TEST(test_sensor_get_latest_returns_zero_initially);
    RUN_TEST(test_sensor_get_latest_out_of_range_returns_zero);
    RUN_TEST(test_lighting_set_and_get_step);
    RUN_TEST(test_lighting_set_all);
    RUN_TEST(test_lighting_out_of_range_ignored);
    return UNITY_END();
}
