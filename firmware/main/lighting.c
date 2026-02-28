#include "lighting.h"

static uint16_t brightness[NUM_STAIRS];

void lighting_init(void)
{
    /* Phase 2: configure I2C and PCA9685 */
}

void lighting_start(void)
{
    /* Phase 2: create FreeRTOS task to push brightness to PCA9685 */
}

void lighting_set_step(uint8_t step, uint16_t value)
{
    if (step >= NUM_STAIRS) {
        return;
    }
    brightness[step] = value;
}

void lighting_set_all(uint16_t value)
{
    for (int step = 0; step < NUM_STAIRS; step++) {
        brightness[step] = value;
    }
}

uint16_t lighting_get_step(uint8_t step)
{
    if (step >= NUM_STAIRS) {
        return 0;
    }
    return brightness[step];
}
