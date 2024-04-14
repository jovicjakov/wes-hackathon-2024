/**
 * @file main.c
 *
 * @brief
 *
 * COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
 * All rights reserved.
 */

//--------------------------------- INCLUDES ----------------------------------
#include "my_mqtt.h"
#include "temp_hum_sensor.h"
#include "tictactoe.h"
#include "joystick.h" #include < stdio.h>
#include "../components/morse/inc/button.h"
#include "../components/led/led.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "../components/buzzer/buzzer.h"
#include "driver/gpio.h"
#include "lis2dh12/lis2dh12.h"

//---------------------------------- MACROS -----------------------------------
#define BUZZER_PIN GPIO_NUM_26
//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------

//------------------------- STATIC DATA & CONSTANTS ---------------------------

static const char *TAG = "MAIN";
//------------------------------- GLOBAL DATA ---------------------------------

//------------------------------ PUBLIC FUNCTIONS -----------------------------
void app_init(void)
{
    esp_err_t ret;

    // Initialize Tic-Tac-Toe
    ret = tictactoe_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize Tic-Tac-Toe: %s", esp_err_to_name(ret));
    }

    // Initialize Temperature Sensor
    ret = temp_sensor_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize temperature sensor: %s", esp_err_to_name(ret));
    }

    // // Initialize PWM
    //  ret = lis_init();
    //  if (ret != ESP_OK) {
    //      ESP_LOGE(TAG, "Failed to initialize ACC SENSOR: %s", esp_err_to_name(ret));
    //  } else {
    //      ESP_LOGE(TAG, "SUCCESS!!");
    //  }

    // Initialize MQTT
    ret = my_mqtt_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize MQTT: %s", esp_err_to_name(ret));
    }

    // Initialize Button
    ret = _button_init(GPIO_BUTTON_1);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize button: %s", esp_err_to_name(ret));
    }

    // Initialize Morse code module
    ret = morse_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize Morse: %s", esp_err_to_name(ret));
    }

    // Initialize LEDs
    ret = led_init(LED_BLUE);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize blue LED: %s", esp_err_to_name(ret));
    }

    ret = led_init(LED_RED);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize red LED: %s", esp_err_to_name(ret));
    }

    // Initialize PWM
    ret = init_pwm();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize PWM: %s", esp_err_to_name(ret));
    }

    // Initialize Joystick
    joystick_init();
    // if (ret != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "Failed to initialize joystick: %s", esp_err_to_name(ret));
    //     // Handle critical error or retry logic
    // }
}

void app_main(void)
{
    app_init();
}

//---------------------------- PRIVATE FUNCTIONS ------------------------------

//---------------------------- INTERRUPT HANDLERS -----------------------------
