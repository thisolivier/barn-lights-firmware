#include "rx_task.h"

#include "config_autogen.h"
#include "status_task.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

_Static_assert(RUN_COUNT <= 4, "RUN_COUNT exceeds supported maximum (4)");

#ifndef UNIT_TEST
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lwip/sockets.h"
#include "esp_log.h"
#else
// Minimal FreeRTOS semaphore stubs for host-side unit tests
typedef int SemaphoreHandle_t;
static inline SemaphoreHandle_t xSemaphoreCreateRecursiveMutex(void) { return 0; }
static inline void xSemaphoreTakeRecursive(SemaphoreHandle_t mutex, int ticks) {
    (void)mutex;
    (void)ticks;
}
static inline void xSemaphoreGiveRecursive(SemaphoreHandle_t mutex) { (void)mutex; }
#define portMAX_DELAY 0
#endif


typedef struct {
    uint32_t frame_id;
    bool run_received[RUN_COUNT];
} FrameSlot;

static FrameSlot frame_slots[2];
static uint8_t **frame_buffers[2];
static int current_slot_index = 0;
static SemaphoreHandle_t frame_mutex;

void rx_task_lock(void) {
    xSemaphoreTakeRecursive(frame_mutex, portMAX_DELAY);
}

void rx_task_unlock(void) {
    xSemaphoreGiveRecursive(frame_mutex);
}

static bool frame_is_newer(uint32_t a, uint32_t b) {
    return (int32_t)(a - b) > 0;
}

static void allocate_buffers(void) {
    for (int slot = 0; slot < 2; ++slot) {
        frame_buffers[slot] = (uint8_t **)malloc(sizeof(uint8_t *) * RUN_COUNT);
        for (int run = 0; run < RUN_COUNT; ++run) {
            frame_buffers[slot][run] = (uint8_t *)malloc(LED_COUNT[run] * 3);
        }
    }
}

static void clear_slot(FrameSlot *slot) {
    slot->frame_id = 0;
    for (int run = 0; run < RUN_COUNT; ++run) {
        slot->run_received[run] = false;
    }
}

void rx_task_process_packet(unsigned int run_index, const uint8_t *data, size_t length) {
    if (run_index >= RUN_COUNT) {
        status_task_increment_drops();
        return;
    }
    size_t expected_length = LED_COUNT[run_index] * 3 + 4;
    if (length != expected_length) {
        status_task_increment_drops();
        return;
    }
    uint32_t frame_id = ((uint32_t)data[0] << 24) |
                        ((uint32_t)data[1] << 16) |
                        ((uint32_t)data[2] << 8) |
                        (uint32_t)data[3];
    rx_task_lock();

    FrameSlot *current_slot = &frame_slots[current_slot_index];
    FrameSlot *next_slot = &frame_slots[1 - current_slot_index];
    FrameSlot *target_slot = NULL;

    if (frame_id == current_slot->frame_id || current_slot->frame_id == 0) {
        current_slot->frame_id = frame_id;
        target_slot = current_slot;
    } else if (frame_id == next_slot->frame_id) {
        target_slot = next_slot;
    } else if (frame_is_newer(frame_id, current_slot->frame_id)) {
        if (next_slot->frame_id == 0 || frame_is_newer(next_slot->frame_id, frame_id)) {
            clear_slot(next_slot);
            next_slot->frame_id = frame_id;
            target_slot = next_slot;
        } else {
            status_task_increment_drops();
            rx_task_unlock();
            return;
        }
    } else {
        status_task_increment_drops();
        rx_task_unlock();
        return;
    }

    status_task_increment_rx_frames();

    size_t payload_length = LED_COUNT[run_index] * 3;
    uint8_t *destination_buffer =
        frame_buffers[target_slot == current_slot ? current_slot_index : 1 - current_slot_index][run_index];
    // Copy payload as-is; driver_task handles any RGB to GRB reordering.
    memcpy(destination_buffer, data + 4, payload_length);

    target_slot->run_received[run_index] = true;

    bool complete = true;
    for (int run = 0; run < RUN_COUNT; ++run) {
        if (!target_slot->run_received[run]) {
            complete = false;
            break;
        }
    }
    if (complete) {
        status_task_increment_complete();
    }
    if (target_slot == next_slot && complete) {
        current_slot_index = 1 - current_slot_index;
        clear_slot(&frame_slots[1 - current_slot_index]);
    }

    rx_task_unlock();
}

#ifndef UNIT_TEST
static void udp_listener_task(void *param) {
    unsigned int run_index = (unsigned int)(uintptr_t)param;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(PORT_BASE + run_index),
    };
    bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    size_t buffer_length = LED_COUNT[run_index] * 3 + 4;
    uint8_t *buffer = (uint8_t *)malloc(buffer_length);
    for (;;) {
        ssize_t received = recvfrom(sock, buffer, buffer_length, 0, NULL, NULL);
        if (received > 0) {
            rx_task_process_packet(run_index, buffer, (size_t)received);
        }
    }
}
#endif

void rx_task_start(void) {
    frame_mutex = xSemaphoreCreateRecursiveMutex();
    allocate_buffers();
    clear_slot(&frame_slots[0]);
    clear_slot(&frame_slots[1]);
#ifndef UNIT_TEST
    for (unsigned int run = 0; run < RUN_COUNT; ++run) {
        xTaskCreatePinnedToCore(udp_listener_task, "rx_run", 4096, (void *)(uintptr_t)run, 5, NULL, 0);
    }
#endif
}

uint32_t rx_task_get_frame_id(int slot_index) {
    if (slot_index < 0 || slot_index > 1) {
        return 0;
    }
    rx_task_lock();
    uint32_t id = frame_slots[slot_index].frame_id;
    rx_task_unlock();
    return id;
}

const uint8_t *rx_task_get_run_buffer(int slot_index, unsigned int run_index) {
    if (slot_index < 0 || slot_index > 1 || run_index >= RUN_COUNT) {
        return NULL;
    }
    rx_task_lock();
    const uint8_t *buffer = frame_buffers[slot_index][run_index];
    rx_task_unlock();
    return buffer;
}

bool rx_task_run_received(int slot_index, unsigned int run_index) {
    if (slot_index < 0 || slot_index > 1 || run_index >= RUN_COUNT) {
        return false;
    }
    rx_task_lock();
    bool received = frame_slots[slot_index].run_received[run_index];
    rx_task_unlock();
    return received;
}

