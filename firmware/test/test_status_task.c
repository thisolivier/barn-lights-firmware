#include "unity.h"
#include "status_task.h"
#include "config_autogen.h"
#include <stdio.h>
#include <string.h>

void setUp(void) { status_task_reset_counters(); }
void tearDown(void) {}

void test_format_json(void) {
    status_task_increment_rx_frames();
    status_task_increment_complete();
    status_task_increment_applied();
    status_task_increment_drops();

    const uint32_t expected_free_total = 111;
    const uint32_t expected_free_internal = 222;
    const uint32_t expected_largest_block = 333;

    char json_buffer[256];
    size_t json_length = status_task_format_json(json_buffer, sizeof(json_buffer), 123, true,
                                                 expected_free_total, expected_free_internal,
                                                 expected_largest_block);

    const char *side_str = SIDE_ID == 0 ? "LEFT" : "RIGHT";
    char expected[256];
    size_t offset = 0;
    offset += snprintf(expected + offset, sizeof(expected) - offset,
                       "{\"id\":\"%s\",\"ip\":\"%u.%u.%u.%u\",\"uptime_ms\":123,\"link\":true,\"runs\":%u,\"leds\":[",
                       side_str, STATIC_IP_ADDR0, STATIC_IP_ADDR1, STATIC_IP_ADDR2, STATIC_IP_ADDR3, RUN_COUNT);
    for (unsigned int run_index = 0; run_index < RUN_COUNT; ++run_index) {
        offset += snprintf(expected + offset, sizeof(expected) - offset, "%u", LED_COUNT[run_index]);
        if (run_index + 1 < RUN_COUNT) {
            offset += snprintf(expected + offset, sizeof(expected) - offset, ",");
        }
    }
    offset += snprintf(expected + offset, sizeof(expected) - offset,
                       "],\"rx_frames\":1,\"complete\":1,\"applied\":1,\"dropped_frames\":1,"
                       "\"mem_free_total\":%u,\"mem_free_internal\":%u,\"mem_largest_block\":%u,\"errors\":[]}",
                       expected_free_total, expected_free_internal, expected_largest_block);

    TEST_ASSERT_EQUAL(offset, json_length);
    TEST_ASSERT_EQUAL_STRING(expected, json_buffer);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_format_json);
    return UNITY_END();
}
