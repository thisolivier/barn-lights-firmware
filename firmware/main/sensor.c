#include "sensor.h"

static sensor_state_t sensor_state;

void sensor_init(void)
{
    /* Phase 2: configure CD4067 mux pins and ADC */
}

void sensor_start(void)
{
    /* Phase 2: create FreeRTOS task for mux scanning */
}

const sensor_state_t *sensor_get_state(void)
{
    return &sensor_state;
}

uint16_t sensor_get_latest(uint8_t step)
{
    if (step >= NUM_STAIRS) {
        return 0;
    }
    return sensor_state.latest[step];
}
