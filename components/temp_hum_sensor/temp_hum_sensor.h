#include "esp_log.h"
#include "driver/i2c.h"
#include "math.h"

#define SDA_IO_NUM 22;
#define SCL_IO_NUM 21;

#define I2C_MASTER_TX_BUF_DISABLE 0 
#define I2C_MASTER_RX_BUF_DISABLE 0
#define WRITE_BIT I2C_MASTER_WRITE            
#define READ_BIT I2C_MASTER_READ
#define ACK_CHECK_EN 0x1
#define ACK_CHECK_DIS 0x0
#define ACK_VAL 0x0
#define NACK_VAL 0x1

typedef struct {
    float temperature;
    float humidity;
} TempHumData;

float getTemperature(void);
float getHumidity(void);
esp_err_t temp_sensor_init(void);
static uint8_t sht31_crc(uint8_t *data);
static esp_err_t sht31_read_temp_humi(float *temp, float *humi);
TempHumData read_temp_humidity(void);

