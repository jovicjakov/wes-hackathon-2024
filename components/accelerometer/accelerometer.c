#include "accelerometer.h"

#define SPI_MISO_IO              19   // Master Input, Slave Output (MISO)
#define SPI_MOSI_IO              23   // Master Output, Slave Input (MOSI)
#define SPI_SCLK_IO              18   // Serial Clock (SCLK)
#define SPI_CS_IO                13   // Chip Select (CS)

#define SPI_CLOCK_SPEED          1000000 // SPI clock speed, 1 MHz

#define LIS2DH12_READ            0x80 // SPI read command
#define LIS2DH12_WRITE           0x00 // SPI write command
#define LIS2DH12_CTRL_REG1       0x20 // Control register 1 address
#define LIS2DH12_OUT_X_L         0x28 // X-axis output register (LSB)
#define LIS2DH12_OUT_X_H         0x29 // X-axis output register (MSB)
#define LIS2DH12_OUT_Y_L         0x2A // Y-axis output register (LSB)
#define LIS2DH12_OUT_Y_H         0x2B // Y-axis output register (MSB)
#define LIS2DH12_OUT_Z_L         0x2C // Z-axis output register (LSB)
#define LIS2DH12_OUT_Z_H         0x2D // Z-axis output register (MSB)

static const char *TAG = "LIS2DH12_SPI";
spi_device_handle_t spi;

esp_err_t spi_master_init() {
    esp_err_t ret;
    spi_bus_config_t buscfg = {
        .miso_io_num = SPI_MISO_IO,
        .mosi_io_num = SPI_MOSI_IO,
        .sclk_io_num = SPI_SCLK_IO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
    };
    ret = spi_bus_initialize(VSPI_HOST, &buscfg, 0); // Use default DMA channel 0
    if (ret != ESP_OK) {
        return ret;
    }
    spi_device_interface_config_t devcfg = {
        .mode = 0,                                // SPI mode 0
        .clock_speed_hz = SPI_CLOCK_SPEED,        // Clock speed
        .spics_io_num = SPI_CS_IO,                // CS pin
        .queue_size = 1,
    };
    ret = spi_bus_add_device(VSPI_HOST, &devcfg, &spi);
    return ret;
}

esp_err_t lis2dh12_write_reg(uint8_t reg_addr, uint8_t data) {
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 16;
    t.flags = SPI_TRANS_USE_TXDATA;
    t.tx_data[0] = LIS2DH12_WRITE | reg_addr;
    t.tx_data[1] = data;
    ret = spi_device_polling_transmit(spi, &t);
    return ret;
}

esp_err_t lis2dh12_read_reg(uint8_t reg_addr, uint8_t *data) {
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 16;
    t.flags = SPI_TRANS_USE_TXDATA;
    t.tx_data[0] = LIS2DH12_READ | reg_addr;
    ret = spi_device_polling_transmit(spi, &t);
    if (ret != ESP_OK) {
        return ret;
    }
    *data = t.rx_data[1]; // Assuming received data is stored in the second byte
    return ret;
}

void lis2dh12_init() {
    esp_err_t ret;
    ret = lis2dh12_write_reg(LIS2DH12_CTRL_REG1, 0x20); // Normal mode, 10Hz data rate
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LIS2DH12");
    }
}

void lis2dh12_read_data() {
    esp_err_t ret;
    uint8_t data[6];
    ret = lis2dh12_read_reg(LIS2DH12_OUT_X_L, &data[0]);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read X-axis data");
        return;
    }
    ret = lis2dh12_read_reg(LIS2DH12_OUT_Y_L, &data[2]);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read Y-axis data");
        return;
    }
    ret = lis2dh12_read_reg(LIS2DH12_OUT_Z_L, &data[4]);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read Z-axis data");
        return;
    }

    // Process data
    // Parse data into acceleration values
    // Example assuming 2's complement 16-bit data format
    int16_t x = (int16_t)(((uint16_t)data[1] << 8) | data[0]);
    int16_t y = (int16_t)(((uint16_t)data[3] << 8) | data[2]);
    int16_t z = (int16_t)(((uint16_t)data[5] << 8) | data[4]);
    float x_g = x * 2.0 / 32768.0; // Convert raw data to g-force (assuming +/- 2g range)
    float y_g = y * 2.0 / 32768.0;
    float z_g = z * 2.0 / 32768.0;

    ESP_LOGI(TAG, "Acceleration (g): X=%.2f, Y=%.2f, Z=%.2f", x_g, y_g, z_g);
}

void accelerometer_main() {
    esp_err_t ret;
    ret = spi_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI");
        return;
    }
    lis2dh12_init();

    while (1) {
        lis2dh12_read_data();
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Read data every second
    }
}