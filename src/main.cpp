#include <Arduino.h>
#include "config_autogen.h"
#include "led_driver.h"
#include "network.h"
#include "receiver.h"
#include "status.h"
#include "led_status.h"

void setup() {
    // Initialize serial for debugging (optional)
    Serial.begin(115200);

    // Initialize LED driver first (sets LEDs black)
    driver_init();

    // Initialize receiver frame assembly
    receiver_init();

    // Initialize network (Ethernet + UDP sockets)
    network_init();

    // Initialize status heartbeat
    status_init();

    // Initialize onboard LED indicator
    led_status_init();

    Serial.println("Teensy LED Controller initialized");
    Serial.print("Side: ");
    Serial.println(SIDE_ID);
    Serial.print("Runs: ");
    Serial.println(RUN_COUNT);
    Serial.print("IP: ");
    Serial.println(network_get_ip_string());
}

void loop() {
    // Poll network for incoming UDP packets
    network_poll();

    // Check if we have a complete frame ready
    if (driver_ready_for_frames()) {
        const uint8_t* frame = receiver_get_complete_frame();
        if (frame != nullptr && !driver_is_busy()) {
            driver_show_frame(frame);
            led_status_frame_displayed();
        }
    }

    // Send heartbeat if interval elapsed
    status_poll();

    // Update onboard LED status
    led_status_poll();
}
