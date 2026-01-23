#include "led_status.h"
#include <Arduino.h>

static const int LED_PIN = 13;
static const uint32_t SLOW_BLINK_INTERVAL_MS = 500;

static bool first_frame_received = false;
static uint32_t frame_count = 0;
static uint32_t last_blink_ms = 0;
static bool led_state = false;

void led_status_init() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    last_blink_ms = millis();
}

void led_status_poll() {
    uint32_t now = millis();

    if (!first_frame_received) {
        // Slow blink until first frame
        if (now - last_blink_ms >= SLOW_BLINK_INTERVAL_MS) {
            last_blink_ms = now;
            led_state = !led_state;
            digitalWrite(LED_PIN, led_state ? HIGH : LOW);
        }
    }
    // After first frame, LED is controlled by led_status_frame_displayed()
}

void led_status_frame_displayed() {
    if (!first_frame_received) {
        first_frame_received = true;
        digitalWrite(LED_PIN, LOW);
    }

    frame_count++;

    // Quick tick every 60th frame for first 600 frames
    if (frame_count <= 600 && (frame_count % 60) == 0) {
        // Brief flash
        digitalWrite(LED_PIN, HIGH);
        delayMicroseconds(1000);  // 1ms flash
        digitalWrite(LED_PIN, LOW);
    }
}
