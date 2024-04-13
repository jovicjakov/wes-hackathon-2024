#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include <string.h>

esp_err_t spi_master_init();
esp_err_t lis2dh12_write_reg(uint8_t reg_addr, uint8_t data);
esp_err_t lis2dh12_read_reg(uint8_t reg_addr, uint8_t *data);
void lis2dh12_init();
void lis2dh12_read_data();
void accelerometer_main();