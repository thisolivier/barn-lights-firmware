#pragma once

#define NUM_STAIRS 16

/* CD4067 mux pins — assign when wiring hardware */
#define MUX_S0_PIN 0  /* GPIO_NUM_XX */
#define MUX_S1_PIN 0  /* GPIO_NUM_XX */
#define MUX_S2_PIN 0  /* GPIO_NUM_XX */
#define MUX_S3_PIN 0  /* GPIO_NUM_XX */
#define MUX_SIG_ADC_CHANNEL 0  /* ADC1_CHANNEL_X */

/* PCA9685 I2C */
#define I2C_SDA_PIN 0  /* GPIO_NUM_XX */
#define I2C_SCL_PIN 0  /* GPIO_NUM_XX */
#define PCA9685_I2C_ADDR 0x40

/* Sensor tuning */
#define SENSOR_SAMPLE_RATE_HZ 200
#define PIEZO_NOISE_FLOOR 50
#define SENSOR_HISTORY_MS 3000

/* Network */
#define WIFI_SSID "CHANGEME"
#define WIFI_PASSWORD "CHANGEME"
#define UDP_CONFIG_PORT 49701
#define UDP_TELEMETRY_PORT 49700
#define UDP_TELEMETRY_DEST_IP "255.255.255.255"
#define UDP_TELEMETRY_RATE_HZ 10
