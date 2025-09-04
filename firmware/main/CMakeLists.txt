idf_component_register(
    SRCS "app_main.c" "net_task.c" "rx_task.c" "driver_task.c" "status_task.c" "startup_sequence.c" "control_task.c" "led_strip_encoder.c"
    INCLUDE_DIRS "." "../include"
    REQUIRES driver
)