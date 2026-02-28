## General directive
- Read 'readme.md' files for the module you are working with.
- Keep code readable by verbose variable names - never abbreviate to a single letter.
- Keep dependencies minimal.
- Prefer simple low-code solutions to complex ones where possible.
- Pro-actively modularize the code
- - Split groups of functions into separate files with clean interfaces.
- - Prefer file lengths of less than 200 lines (light preference).
- - Add readme.md files at the root of each module to describe the architecture and subcomponents.
- - Ensure readme files are updated at the end of each task.

Your work is deeply appreciated.

## Project context
- This is a stair lights firmware project for ESP32 using ESP-IDF.
- The `udp_comms` component is a reusable module extracted from the barn-lights project.
- Application code (sensor, lighting, protocol) lives in `firmware/main/`.
- Host-side tests use Unity and compile with `-DUNIT_TEST` to stub ESP-IDF dependencies.

## Testing
- Run host tests: `cmake -S firmware/test -B firmware/test/build && cmake --build firmware/test/build`
- Execute: `./firmware/test/build/test_udp_comms` and `./firmware/test/build/test_protocol`
