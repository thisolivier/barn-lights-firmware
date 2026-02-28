idf_component_register(
    SRCS "main.c" "protocol.c" "sensor.c" "lighting.c"
    INCLUDE_DIRS "."
    REQUIRES udp_comms
)
