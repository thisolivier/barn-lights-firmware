#pragma once

#include <stdint.h>
#include "app_config.h"

typedef struct {
    uint16_t latest[NUM_STAIRS];
    uint16_t peak[NUM_STAIRS];
} sensor_state_t;

void sensor_init(void);
void sensor_start(void);
const sensor_state_t *sensor_get_state(void);
uint16_t sensor_get_latest(uint8_t step);
