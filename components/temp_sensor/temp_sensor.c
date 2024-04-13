#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "temp_sensor.h"

static const char *TAG = "temp_sensor";
static i2c_port_t i2c_port = I2C_NUM_1;
static float temp, humi;

esp_err_t temp_sensor_init(void)
{
    ESP_LOGI(TAG, "11111111111111111111111111111111111111111111");
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA_IO_NUM;
    ESP_LOGI(TAG, "22222222222222222222222222222222222222222222");
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = SCL_IO_NUM;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    ESP_LOGI(TAG, "33333333333333333333333333333333333333333333");
    conf.master.clk_speed = 10000;
    conf.clk_flags = 0;
    ESP_LOGI(TAG, "44444444444444444444444444444444444444444444");
    i2c_param_config(i2c_port, &conf);
    ESP_LOGI(TAG, "55555555555555555555555555555555555555555555");
    return i2c_driver_install(i2c_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

void temp_sensor_main(void)
{  
    ESP_ERROR_CHECK(temp_sensor_init());
      ESP_ERROR_CHECK(sht31_read_temp_humi(&temp, &humi));

      ESP_LOGI(TAG, "temp = %.2f, humi = %.2f\n", temp, humi);
      vTaskDelay(2000 / portTICK_PERIOD_MS); 

}

static uint8_t sht31_crc(uint8_t *data) {

    uint8_t crc = 0xff;
    int i, j;
    for(i = 0; i < 2; i++) {
        crc ^= data[i];
        for(j = 0; j < 8; j++) {
            if(crc & 0x80) {
                crc <<= 1;
                crc ^= 0x131;
            }
            else
                crc <<= 1;
        }
    }
    return crc;
}

static esp_err_t sht31_read_temp_humi(float *temp, float *humi)
{
    ESP_LOGI(TAG, "11111111111111111111111111111111111111111111111111111111111111111");
    // See http://wiki.seeedstudio.com/Grove-TempAndHumi_Sensor-SHT31/
    // Write 0x00 to address 0x24
    unsigned char data_wr[] = {0x24, 0x00};
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (0x44 << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, data_wr, sizeof(data_wr), ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ESP_LOGI(TAG, "222222222222222222222222222222222222222222222222222222222222222222");
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "3333333333333333333333333333333333333333333333333333333333333333333");
    if (ret != ESP_OK) {
        return ret;
    }
    ESP_LOGI(TAG, "44444444444444444444444444444444444444444444444444444444444444444444");
    i2c_cmd_link_delete(cmd);

    
    // Delay 20 ms
    vTaskDelay(2);

    // Read 6 bytes
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (0x44 << 1) | READ_BIT, ACK_CHECK_EN);
    size_t size = 6;
    uint8_t *data_rd = malloc(size);
    if (size > 1) {
        i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        return ret;
    }
    i2c_cmd_link_delete(cmd);

    // check error
    if(data_rd[2] != sht31_crc(data_rd) || 
       data_rd[5] != sht31_crc(data_rd + 3)) 
        return ESP_ERR_INVALID_CRC;
    
    *temp = -45 + (175 *(float)(data_rd[0] * 256 + data_rd[1])/ 65535.0);
    *humi = 100 * (float)(data_rd[3] * 256 + data_rd[4]) / 65535.0;

    return ESP_OK;
}

void getTemperature(void) {
    ESP_ERROR_CHECK(sht31_read_temp_humi(&temp, &humi));
    ESP_LOGI(TAG, "Temperature: %f", temp);
    //return temp;
}

void getHumidity(void) {
    ESP_ERROR_CHECK(sht31_read_temp_humi(&temp, &humi));
    ESP_LOGI(TAG, "Humidity: %f%%", humi);
    //return humi;
}