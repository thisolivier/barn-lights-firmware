# UDP Data Format — Stair Lights

Two independent UDP channels carry data between a laptop and the ESP32.

## Config channel (laptop -> ESP32)

Port: `49701` (configurable via `UDP_CONFIG_PORT`)

| Offset | Size | Description |
|--------|------|-------------|
| 0 | 1 | Command type |
| 1 | N | Command payload |

### Command types

| Type | Name | Payload |
|------|------|---------|
| 0x01 | SET_STEP_BRIGHTNESS | step_index (1 byte) + brightness (2 bytes big-endian) |
| 0x02 | SET_ALL_BRIGHTNESS | brightness (2 bytes big-endian) |
| 0x03 | SET_THRESHOLD | step_index (1 byte) + threshold (2 bytes big-endian) |
| 0x04 | SET_TELEMETRY_RATE | interval_ms (4 bytes big-endian) |

## Telemetry channel (ESP32 -> laptop)

Port: `49700` (configurable via `UDP_TELEMETRY_PORT`)

Sent at a configurable rate (default 10 Hz). 104 bytes per packet.

| Offset | Size | Description |
|--------|------|-------------|
| 0-3 | 4 | uptime_ms (big-endian) |
| 4-5 | 2 | sequence_number (big-endian) |
| 6-7 | 2 | sample_rate_hz (big-endian) |
| 8-39 | 32 | latest_adc_values[16] (2 bytes each, big-endian) |
| 40-71 | 32 | peak_adc_values[16] (2 bytes each, big-endian) |
| 72-103 | 32 | brightness_values[16] (2 bytes each, big-endian) |

All multi-byte integers are unsigned big-endian.
