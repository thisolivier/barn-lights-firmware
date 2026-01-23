#include "led_driver.h"
#include "config_autogen.h"
#include <OctoWS2811.h>
#include <Arduino.h>

// OctoWS2811 always drives 8 outputs in parallel
// Buffer size is MAX_LEDS per output * 8 outputs
static const int LEDS_PER_STRIP = MAX_LEDS;
static const int NUM_STRIPS = 8;

// OctoWS2811 requires display memory (double-buffered internally)
DMAMEM static int display_memory[LEDS_PER_STRIP * 6];
static int drawing_memory[LEDS_PER_STRIP * 6];

static OctoWS2811 leds(LEDS_PER_STRIP, display_memory, drawing_memory,
                       WS2811_GRB | WS2811_800kHz);

static uint32_t startup_time_ms = 0;
static const uint32_t STARTUP_BLACKOUT_MS = 1000;

void driver_init() {
    leds.begin();
    startup_time_ms = millis();

    // Set all LEDs to black initially
    driver_show_black();
}

void driver_show_frame(const uint8_t* frame_data) {
    // Frame data is RGB, need to copy to OctoWS2811 buffer
    // Frame layout: run0 data, run1 data, run2 data, ...
    // Each run has LED_COUNT[run] * 3 bytes (RGB)

    const uint8_t* src = frame_data;

    for (int run = 0; run < RUN_COUNT; run++) {
        int led_count = LED_COUNT[run];

        for (int i = 0; i < led_count; i++) {
            uint8_t r = *src++;
            uint8_t g = *src++;
            uint8_t b = *src++;

            // OctoWS2811 setPixel takes strip number and pixel index
            // Color is packed as 0x00RRGGBB (OctoWS2811 handles GRB conversion internally)
            int color = (r << 16) | (g << 8) | b;
            leds.setPixel(run * LEDS_PER_STRIP + i, color);
        }

        // Clear any remaining LEDs in this strip (beyond LED_COUNT[run])
        for (int i = led_count; i < LEDS_PER_STRIP; i++) {
            leds.setPixel(run * LEDS_PER_STRIP + i, 0);
        }
    }

    // Clear unused strips
    for (int run = RUN_COUNT; run < NUM_STRIPS; run++) {
        for (int i = 0; i < LEDS_PER_STRIP; i++) {
            leds.setPixel(run * LEDS_PER_STRIP + i, 0);
        }
    }

    leds.show();
}

void driver_show_black() {
    for (int i = 0; i < LEDS_PER_STRIP * NUM_STRIPS; i++) {
        leds.setPixel(i, 0);
    }
    leds.show();
}

bool driver_is_busy() {
    return leds.busy();
}

bool driver_ready_for_frames() {
    return (millis() - startup_time_ms) >= STARTUP_BLACKOUT_MS;
}
