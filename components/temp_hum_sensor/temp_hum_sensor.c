#include "temp_hum_sensor.h"

static const char *TAG = "SENSORS";
static i2c_port_t i2c_port = I2C_NUM_1;

static void temp_hum_sensor_task(void *pvParameters);

QueueHandle_t temperature_change_queue = NULL;

extern TempHumData;

esp_err_t temp_sensor_init(void) {
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA_IO_NUM;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = SCL_IO_NUM;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 10000;
    conf.clk_flags = 0;

    ESP_ERROR_CHECK(i2c_param_config(i2c_port, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(i2c_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0));

    // Create a task to continuously read from the sensor
    xTaskCreate(temp_hum_sensor_task, "TempHum_Sensor_Task", 2048, NULL, 5, NULL);

    temperature_change_queue = xQueueCreate(20, sizeof(TempHumData));
    if (temperature_change_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create the queue");
        // Handle error
    }

    return ESP_OK;
}

static void temp_hum_sensor_task(void *pvParameters) {
    static float last_temp = 0.0;  // Stores the last temperature reading
    bool firstRead = true;  // Flag to track if it's the first sensor reading
    TempHumData data;  // Struct to store the current temperature and humidity

    while (1) {
        if (sht31_read_temp_humi(&data.temperature, &data.humidity) == ESP_OK) {
            // Check if it's the first read or if the change in temperature exceeds 0.05°C
            if (firstRead || fabs(data.temperature - last_temp) >= 0.05) {
                // Send data to the queue if it's not null
                if (temperature_change_queue != NULL) {
                    if (xQueueSend(temperature_change_queue, &data, 0) != pdPASS) {
                        //ESP_LOGE(TAG, "Failed to send temperature and humidity to queue");
                    } else {
                        //ESP_LOGI(TAG, "Succesfully pushed to temperature_change_queue!");
                        ESP_LOGI(TAG, "SENSOR READ: Temperature: %.2f°C, Humidity: %.2f%%", data.temperature, data.humidity);
                    }
                }
                // Update last_temp with the current temperature
                last_temp = data.temperature;
                // Clear the firstRead flag after the first successful send
                firstRead = false;
            }
            //ESP_LOGI(TAG, "Temperature: %.2f°C, Humidity: %.2f%%", data.temperature, data.humidity);
        } else {
            ESP_LOGE(TAG, "Failed to read temperature and humidity from sensor");
        }
        // Sleep for 10 seconds before the next reading
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
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

    // See http://wiki.seeedstudio.com/Grove-TempAndHumi_Sensor-SHT31/
    // Write 0x00 to address 0x24
    unsigned char data_wr[] = {0x24, 0x00};
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (0x44 << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, data_wr, sizeof(data_wr), ACK_CHECK_EN);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    
    if (ret != ESP_OK) {
        return ret;
    }

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

TempHumData read_temp_humidity(void) {
    TempHumData data;
    data.temperature = -999;
    data.humidity = -999;
    if (sht31_read_temp_humi(&data.temperature, &data.humidity) == ESP_OK) {
        return data;
    } else {
        ESP_LOGE(TAG, "Failed to read temperature");
        return data; // Return an error value
    }
}
