#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * Config rx callback — passed to udp_comms as config_handler.
 * Parses command type and dispatches to sensor/lighting.
 */
void protocol_handle_config(const uint8_t *data, size_t length);

/**
 * Telemetry tx callback — passed to udp_comms as telemetry_packer.
 * Packs sensor readings, peak values, and brightness into buffer.
 * Returns bytes written.
 */
size_t protocol_pack_telemetry(uint8_t *buffer, size_t max_length);
