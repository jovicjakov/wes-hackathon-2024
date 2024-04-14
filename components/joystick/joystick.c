#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <sys/time.h>

#include "soc/soc_caps.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"

#include "driver/gpio.h"
#include "sdkconfig.h"

#if CONFIG_IDF_TARGET_ESP32
#define EXAMPLE_ADC1_CHAN0 ADC_CHANNEL_6
#define EXAMPLE_ADC1_CHAN1 ADC_CHANNEL_7
#define EXAMPLE_ADC_ATTEN ADC_ATTEN_DB_11
#endif

static char const *TAG = "JOYSTICK";

static int adc_raw[2];
static adc_oneshot_unit_handle_t adc1_handle;
static TaskHandle_t joystick_task_handle = NULL;
extern QueueHandle_t joystick_to_gui_queue;

enum inputs
{
    INPUT_PUSH_BUTTON,
    INPUT_UP_ARROW,
    INPUT_DOWN_ARROW,
    INPUT_RIGHT_ARROW,
    INPUT_LEFT_ARROW,
    INPUT_BACK_TO_CENTRE
};

static int last_input = INPUT_BACK_TO_CENTRE; // Default to centre position at start

// Forward declarations for calibration functions
static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
static void example_adc_calibration_deinit(adc_cali_handle_t handle);

static void joystick_task(void *pvParameters);
void joystick_init(void);
void inputHandler(int input);

void joystick_init(void)
{
    adc_oneshot_unit_init_cfg_t init_config1 = {.unit_id = ADC_UNIT_1};
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = EXAMPLE_ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN0, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN1, &config));

    adc_cali_handle_t adc1_cali_chan0_handle = NULL, adc1_cali_chan1_handle = NULL;
    bool calibrated_chan0 = example_adc_calibration_init(ADC_UNIT_1, EXAMPLE_ADC1_CHAN0, EXAMPLE_ADC_ATTEN, &adc1_cali_chan0_handle);
    bool calibrated_chan1 = example_adc_calibration_init(ADC_UNIT_1, EXAMPLE_ADC1_CHAN1, EXAMPLE_ADC_ATTEN, &adc1_cali_chan1_handle);

    if (joystick_task_handle == NULL)
    {
        xTaskCreate(joystick_task, "joystick_task", 2048, NULL, 5, NULL);
    }
}

void inputHandler(int input)
{
    switch (input)
    {
    case INPUT_UP_ARROW:
        ESP_LOGI(TAG, "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^UP");
        break;
    case INPUT_DOWN_ARROW:
        ESP_LOGI(TAG, "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||DOWN");
        break;
    case INPUT_LEFT_ARROW:
        ESP_LOGI(TAG, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<LEFT");
        int left = -1;
        xQueueSend(joystick_to_gui_queue, &left, 0U);
        break;
    case INPUT_RIGHT_ARROW:
        ESP_LOGI(TAG, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>RIGHT");
        int right = 1;
        xQueueSend(joystick_to_gui_queue, &right, 0U);
        break;
    case INPUT_BACK_TO_CENTRE:
        ESP_LOGI(TAG, "================================================================================centre");
        break;
    default:
        break;
    }
}

static void joystick_task(void *pvParameters) {
    int current_input = INPUT_BACK_TO_CENTRE;

    while (true) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN0, &adc_raw[0]));
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN1, &adc_raw[1]));

        // Determine the current joystick input based on ADC readings
        if (adc_raw[0] <= 1000) {
            current_input = INPUT_RIGHT_ARROW;
        } else if (adc_raw[0] >= 3500) {
            current_input = INPUT_LEFT_ARROW;
        } else if (adc_raw[1] <= 500) {
            current_input = INPUT_UP_ARROW;
        } else if (adc_raw[1] >= 4000) {
            current_input = INPUT_DOWN_ARROW;
        } else {
            current_input = INPUT_BACK_TO_CENTRE;
        }

        // Call the input handler only if the state has changed
        if (current_input != last_input) {
            inputHandler(current_input);
            last_input = current_input; // Update the last input to the current input
        }

        // Regular delay to ensure responsiveness and WDT feeding
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------*/
static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated)
    {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK)
        {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated)
    {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK)
        {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Calibration Success");
    }
    else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated)
    {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    }
    else
    {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

static void example_adc_calibration_deinit(adc_cali_handle_t handle)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
#endif
}