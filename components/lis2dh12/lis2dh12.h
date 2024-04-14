#ifndef _LIS2HH12TR_H
#define _LIS2HH12TR_H

#include "freertos/FreeRTOS.h"
#include <stdio.h>
#include "driver/i2c.h"

/****************************** ERRORS ******************************/
#define LIS2HH12_OK true     // No Error
#define LIS2HH12_ERROR false // Error Exists

#define LIS2HH12_NO_ERROR 0   // Initial State
#define LIS2HH12_READ_ERROR 1 // Accelerometer Reading Error
#define LIS2HH12_BAD_ARG 2    // Bad Argument

#define LIS2DH12_RANGE_2GA 0x00
#define LIS2DH12_RANGE_4GA 0x10
#define LIS2DH12_RANGE_8GA 0x20
#define LIS2DH12_RANGE_16GA 0x30

typedef struct
{
    uint8_t device_address;
    int port_num;
    uint8_t range;
    uint8_t mg_scale;
} lis_t;

typedef lis_t *lis_handle_t;

typedef struct
{
    gpio_num_t sda_pin;
    gpio_num_t scl_pin;
    uint8_t device_address;
    i2c_port_t port_num;
    uint8_t range;
} lis_config_t;

esp_err_t lis_init(void);
esp_err_t lis_read_accel_xyz(lis_handle_t lis_handle, int16_t *x, int16_t *y, int16_t *z);
void lis_mg_scale(lis_handle_t lis_handle, int16_t *x, int16_t *y, int16_t *z);

#endif