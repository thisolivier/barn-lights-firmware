#pragma once

#include <stdbool.h>
#include <stdint.h>

// Maximum backward difference to treat as a transmitter restart.
// Differences greater than this are considered a reset and thus newer.
#define FRAME_RESET_THRESHOLD 1000U

static inline bool frame_is_newer_with_reset(uint32_t candidate_frame_id,
                                             uint32_t reference_frame_id,
                                             uint32_t reset_threshold)
{
    int32_t diff = (int32_t)(candidate_frame_id - reference_frame_id);
    return diff > 0 || diff < -(int32_t)reset_threshold;
}

