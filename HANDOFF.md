# Phase 2 Handoff — Stair Lights Firmware

## Branch
`claude/udp-comms-stair-lights-rTRvP`

## What's Done (Phase 1)
All barn-lights code removed. New stair-lights skeleton in place:

- **`firmware/components/udp_comms/`** — Reusable ESP-IDF component. WiFi STA with 10 retries, config rx task (UDP listen + app callback), telemetry tx task (periodic UDP send + app packer callback). Fully working, 9 unit tests passing.
- **`firmware/main/`** — `main.c` wires udp_comms to stub callbacks. `protocol.c/.h` (config parse + telemetry pack stubs), `sensor.c/.h` (state struct + accessors), `lighting.c/.h` (brightness array with set/get). `app_config.h` has all constants with GPIO placeholders.
- **`firmware/test/`** — 16 Unity tests passing. CMake + FetchContent, `UNIT_TEST` define stubs ESP-IDF.
- **`docs/udp-data-format.md`** — Binary protocol spec (104-byte telemetry, 4 config commands).

Run tests to confirm baseline:
```bash
cmake -S firmware/test -B firmware/test/build && cmake --build firmware/test/build
./firmware/test/build/test_udp_comms
./firmware/test/build/test_protocol
```

## What's Next (Phase 2)

### Plan
1. **Review existing code** — read through all Phase 1 files to understand current state
2. **Use Context7 MCP to verify ESP-IDF v5.x API signatures** before writing any hardware driver code
3. **Implement Phase 2 files:**
   - `protocol.c` — Parse 4 config commands, pack 104-byte telemetry struct
   - `sensor.c` — CD4067 mux GPIO + ADC oneshot, 200Hz FreeRTOS scan task, rolling peak buffer
   - `lighting.c` — I2C master + PCA9685 register writes, FreeRTOS push task
   - `tools/monitor.py` — Telemetry listener + config sender
   - Tests — Protocol round-trip, sensor buffer logic

### Critical: Use Context7 MCP to verify ESP-IDF APIs

The barn project used **ESP-IDF v5.x** APIs. Key APIs to check with Context7 before writing code:

| Module | API to verify | Why |
|---|---|---|
| `sensor.c` | `adc_oneshot_new_unit()`, `adc_oneshot_config_channel()`, `adc_oneshot_read()` | v5.x oneshot ADC driver replaced v4.x `adc1_get_raw()` |
| `lighting.c` | `i2c_master_bus_new()`, `i2c_master_bus_add_device()`, `i2c_master_transmit()` | v5.x I2C master driver replaced v4.x `i2c_master_write_to_device()` |
| `udp_comms.c` | WiFi APIs are stable, already written and correct | No changes needed |

GPIO APIs (`gpio_set_level` for mux pins) are stable across versions.

### Open Design Questions
1. **PCA9685** — Write I2C register-level driver from scratch (~80 lines) or use a library?
2. **ADC attenuation** — What voltage range do the piezos output? Determines `ADC_ATTEN_DB_0` (0-1.1V) vs `ADC_ATTEN_DB_11` (0-3.3V)
3. **Mux settling time** — Defaulting to 10us, adjustable via `app_config.h`
4. **Thread safety** — Atomic single-word reads/writes (no mutex) OK for MVP?

## File Map

```
firmware/
├── main/
│   ├── main.c              Entry point, wires udp_comms callbacks
│   ├── app_config.h        All constants (GPIO placeholders, network, tuning)
│   ├── protocol.c/.h       Config parse + telemetry pack (STUB — Phase 2)
│   ├── sensor.c/.h         CD4067 mux + ADC (STUB — Phase 2)
│   └── lighting.c/.h       PCA9685 I2C (STUB — Phase 2, set/get works)
├── components/
│   └── udp_comms/          Reusable WiFi + UDP component (DONE)
│       ├── include/udp_comms.h
│       └── udp_comms.c
└── test/                   Host-side Unity tests (16 passing)
    ├── CMakeLists.txt
    ├── test_udp_comms.c
    └── test_protocol.c
```
