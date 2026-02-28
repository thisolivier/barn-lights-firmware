# Claude Configuration for Stair Lights Firmware

## Project Overview

- This is a stair lights firmware project for ESP32 using ESP-IDF.
- The `udp_comms` component is a reusable module extracted from the barn-lights project.
- Application code (sensor, lighting, protocol) lives in `firmware/main/`.
- Host-side tests use Unity and compile with `-DUNIT_TEST` to stub ESP-IDF dependencies.

## General Directives
- Read 'readme.md' files for the module you are working with
- Keep code readable with verbose variable names — never abbreviate to a single letter
- Keep dependencies minimal (balance with low code directive)
- Prefer simple low-code solutions to complex ones where possible (balance with minimal dependency directive)
- When working on complex tasks, always use a todo list
- When working on tasks which involve multiple stages or components, use the following strategies
  - Always work using sub-agents and report progress back to the main agent
  - Use a working tree for new major branches
  - Establish what can be run in parallel with sub-agents and what is blocking
  - Review the todo list and parallel/sequential strategy after each few tasks complete
- Use the `context7` skill proactively for library/API documentation — don't wait to be asked
  - Especially important for ESP-IDF v5.x APIs, which may differ from training data
- ALWAYS ask for permission before editing this file

Your work is deeply appreciated.

## Codebase Health
- Pro-actively modularize the code
  - Split groups of functions into separate files with clean interfaces
  - Prefer file lengths of less than 200 lines (light preference)
- Keep documentation up to date
  - Add readme.md files at the root of each module
  - Readme files should briefly describe module architecture and working patterns
  - Readme files should list the main components and their purposes
  - Ensure readme files are up to date at the end of each task
  - Readme files do not need to list every feature of the code, just the broad strokes
- Write tests for new features
- Ensure all tests can be run with a single command

## Testing
- Run host tests: `cmake -S firmware/test -B firmware/test/build && cmake --build firmware/test/build`
- Execute: `./firmware/test/build/test_udp_comms` and `./firmware/test/build/test_protocol`

## Working Trees

When asked to work in a new git working tree:
1. Create the working tree inside the `working-trees/` directory at the repository root
2. Each working tree directory should be locked to a single Claude session at a time
3. Before using a working tree, check if it's already in use by another session
4. Use a lock file (e.g., `.claude-session.lock`) in the working tree to indicate active use
5. Release the lock when the session ends or when switching away from that working tree

## Paths Not to Modify
- Do not modify files managed by build systems or package managers
- Avoid editing top-level `CMakeLists.txt` and `sdkconfig` unless the task explicitly requires it
