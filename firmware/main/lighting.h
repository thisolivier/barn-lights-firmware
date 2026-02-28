#pragma once

#include <stdint.h>
#include "app_config.h"

void lighting_init(void);
void lighting_start(void);
void lighting_set_step(uint8_t step, uint16_t brightness);
void lighting_set_all(uint16_t brightness);
uint16_t lighting_get_step(uint8_t step);
