#include "unity.h"
#include "frame_utils.h"

void setUp(void) {}
void tearDown(void) {}

void test_detects_newer_frame(void) {
    TEST_ASSERT_TRUE(frame_is_newer_with_reset(2, 1, FRAME_RESET_THRESHOLD));
    TEST_ASSERT_FALSE(frame_is_newer_with_reset(1, 2, FRAME_RESET_THRESHOLD));
}

void test_detects_restart(void) {
    // Large backward jump indicates transmitter restart.
    TEST_ASSERT_TRUE(frame_is_newer_with_reset(1, 5000, FRAME_RESET_THRESHOLD));
    // Small backward jump treated as out-of-order and ignored.
    TEST_ASSERT_FALSE(frame_is_newer_with_reset(4990, 5000, FRAME_RESET_THRESHOLD));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_detects_newer_frame);
    RUN_TEST(test_detects_restart);
    return UNITY_END();
}
