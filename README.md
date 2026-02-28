# Stair Lights Firmware

ESP-IDF firmware for an ESP32 driving 16 stair-step piezo sensors and LED strips over WiFi, with UDP-based telemetry and config.

## Architecture

```
firmware/
├── main/           Application code (sensor, lighting, protocol, main)
├── components/
│   └── udp_comms/  Reusable WiFi + UDP comms component (config rx, telemetry tx)
└── test/           Host-side unit tests (Unity)
tools/              Laptop-side utilities (monitor.py)
docs/               Protocol and data format documentation
```

### udp_comms component

A self-contained ESP-IDF component providing:
- WiFi STA connection with retry
- Config inbound channel: listens on a UDP port, calls an app callback with raw payload
- Telemetry outbound channel: at a configurable interval, calls an app callback to fill a buffer, sends it as UDP

The component has no knowledge of stairs, piezos, or LEDs. It moves bytes.

## Development

### Run host-side tests

```bash
cmake -S firmware/test -B firmware/test/build
cmake --build firmware/test/build
./firmware/test/build/test_udp_comms
./firmware/test/build/test_protocol
```

### Build firmware (requires ESP-IDF)

```bash
. ~/esp/esp-idf/export.sh
cd firmware
idf.py set-target esp32
idf.py build
```

### Flash

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

## Documentation

- [UDP data format](docs/udp-data-format.md)
