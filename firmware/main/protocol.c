#include "protocol.h"
#include "app_config.h"

void protocol_handle_config(const uint8_t *data, size_t length)
{
    /* Phase 2: parse command type byte and dispatch */
    (void)data;
    (void)length;
}

size_t protocol_pack_telemetry(uint8_t *buffer, size_t max_length)
{
    /* Phase 2: pack sensor + lighting state into binary telemetry */
    (void)buffer;
    (void)max_length;
    return 0;
}
