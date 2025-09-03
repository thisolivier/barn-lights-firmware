#include "driver_task.h"

#include "config_autogen.h"
#include "rx_task.h"
#include "status_task.h"
#include "startup_sequence.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"
#include "led_strip_encoder.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_rom_sys.h"

#include <stdlib.h>
#include <string.h>

#define RMT_CLK_DIV 2

#define RUN0_GPIO 12
#define RUN1_GPIO 13
#define RUN2_GPIO 14
#define RUN3_GPIO 15

static const gpio_num_t RUN_GPIO[] = {
    RUN0_GPIO
#if RUN_COUNT > 1
    , RUN1_GPIO
#endif
#if RUN_COUNT > 2
    , RUN2_GPIO
#endif
#if RUN_COUNT > 3
    , RUN3_GPIO
#endif
};

_Static_assert(sizeof(RUN_GPIO) / sizeof(RUN_GPIO[0]) == RUN_COUNT,
               "RUN_COUNT mismatch");
_Static_assert(RUN_COUNT <= SOC_RMT_CHANNELS_PER_GROUP,
               "Too many runs for available RMT channels");
_Static_assert(RUN_COUNT <= 4, "RUN_COUNT exceeds supported maximum (4)");
_Static_assert(RUN0_GPIO >= 0 && RUN0_GPIO <= 39, "RUN0_GPIO out of range");
#if RUN_COUNT > 1
_Static_assert(RUN1_GPIO >= 0 && RUN1_GPIO <= 39, "RUN1_GPIO out of range");
#endif
#if RUN_COUNT > 2
_Static_assert(RUN2_GPIO >= 0 && RUN2_GPIO <= 39, "RUN2_GPIO out of range");
#endif
#if RUN_COUNT > 3
_Static_assert(RUN3_GPIO >= 0 && RUN3_GPIO <= 39, "RUN3_GPIO out of range");
#endif


static uint8_t *grb_buffers[RUN_COUNT];
static rmt_channel_handle_t rmt_channels[RUN_COUNT];
static rmt_encoder_handle_t led_encoder;
static const rmt_transmit_config_t TRANSMIT_CONFIG = {
    .loop_count = 0,
};

static esp_err_t wait_all_done_retry(rmt_channel_handle_t channel) {
    const int MAX_ATTEMPTS = 5;
    for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt) {
        esp_err_t err = rmt_tx_wait_all_done(channel, pdMS_TO_TICKS(1));
        if (err == ESP_OK) {
            return ESP_OK;
        }
        if (err != ESP_ERR_TIMEOUT) {
            return err;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    ESP_LOGW("driver_task", "rmt_tx_wait_all_done timeout");
    return ESP_ERR_TIMEOUT;
}

static inline void convert_run_to_grb(unsigned int run_index, const uint8_t *rgb_data)
{
    uint8_t *grb = grb_buffers[run_index];
    for (unsigned int led_index = 0; led_index < LED_COUNT[run_index]; ++led_index) {
        uint8_t red = rgb_data[led_index * 3];
        uint8_t green = rgb_data[led_index * 3 + 1];
        uint8_t blue = rgb_data[led_index * 3 + 2];
        grb[led_index * 3] = green;
        grb[led_index * 3 + 1] = red;
        grb[led_index * 3 + 2] = blue;
    }
}

static bool frame_is_newer(uint32_t a, uint32_t b)
{
    return (int32_t)(a - b) > 0;
}

static void send_frame(int slot_index)
{
    // Protect buffers while we read/convert
    rx_task_lock();
    for (unsigned int run = 0; run < RUN_COUNT; ++run) {
        const uint8_t *buffer = rx_task_get_run_buffer(slot_index, run);
        convert_run_to_grb(run, buffer);
    }
    rx_task_unlock();

    for (unsigned int run = 0; run < RUN_COUNT; ++run) {
        ESP_ERROR_CHECK(rmt_transmit(
            rmt_channels[run],
            led_encoder,
            grb_buffers[run],
            LED_COUNT[run] * 3,
            &TRANSMIT_CONFIG));
    }
    for (unsigned int run = 0; run < RUN_COUNT; ++run) {
        wait_all_done_retry(rmt_channels[run]);
    }
}


static void send_black(void)
{
    for (unsigned int run_index = 0; run_index < RUN_COUNT; ++run_index) {
        size_t byte_count = LED_COUNT[run_index] * 3;
        memset(grb_buffers[run_index], 0, byte_count);
        ESP_ERROR_CHECK(rmt_transmit(rmt_channels[run_index], led_encoder,
                                     grb_buffers[run_index], byte_count,
                                     &TRANSMIT_CONFIG));
    }
    for (unsigned int run_index = 0; run_index < RUN_COUNT; ++run_index) {
        wait_all_done_retry(rmt_channels[run_index]);
        esp_rom_delay_us(60);
    }
}

static void flash_run(unsigned int run_index,
                      uint8_t red, uint8_t green, uint8_t blue)
{
    size_t led_count = LED_COUNT[run_index];
    uint8_t *buffer = grb_buffers[run_index];
    for (unsigned int led = 0; led < led_count; ++led) {
        buffer[led * 3] = green;
        buffer[led * 3 + 1] = red;
        buffer[led * 3 + 2] = blue;
    }
    ESP_ERROR_CHECK(rmt_transmit(rmt_channels[run_index], led_encoder,
                                 buffer, led_count * 3,
                                 &TRANSMIT_CONFIG));
    wait_all_done_retry(rmt_channels[run_index]);
}

static void delay_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static void driver_task(void *arg)
{
    led_strip_encoder_config_t encoder_config = {
        .resolution = 80000000 / RMT_CLK_DIV,
    };
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

    for (unsigned int run = 0; run < RUN_COUNT; ++run) {
        rmt_tx_channel_config_t channel_config = {
            .gpio_num = RUN_GPIO[run],
            .clk_src = RMT_CLK_SRC_DEFAULT,
            .resolution_hz = 80000000 / RMT_CLK_DIV,
            .mem_block_symbols = 64,
            .trans_queue_depth = 1,
        };
        ESP_ERROR_CHECK(rmt_new_tx_channel(&channel_config, &rmt_channels[run]));
        ESP_ERROR_CHECK(rmt_enable(rmt_channels[run]));
        grb_buffers[run] = (uint8_t *)malloc(LED_COUNT[run] * 3);
        memset(grb_buffers[run], 0, LED_COUNT[run] * 3);
    }

    send_black();
    startup_sequence(RUN_COUNT, flash_run, send_black, delay_ms);

    uint32_t last_frame_id = 0;

    for (;;) {
        int selected_slot = -1;
        uint32_t selected_id = last_frame_id;
        rx_task_lock();
        for (int slot = 0; slot < 2; ++slot) {
            uint32_t frame_id = rx_task_get_frame_id(slot);
            if (frame_is_newer(frame_id, selected_id)) {
                bool frame_complete = true;
                for (unsigned int run = 0; run < RUN_COUNT; ++run) {
                    if (!rx_task_run_received(slot, run)) {
                        frame_complete = false;
                        break;
                    }
                }
                if (frame_complete) {
                    selected_slot = slot;
                    selected_id = frame_id;
                }
            }
        }
        rx_task_unlock();

        if (selected_slot >= 0) {
            send_frame(selected_slot);
            status_task_increment_applied();
            last_frame_id = selected_id;
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void driver_task_start(void)
{
    xTaskCreate(driver_task, "driver_task", 4096, NULL, 5, NULL);
}

