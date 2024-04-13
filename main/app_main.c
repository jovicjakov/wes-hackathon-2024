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
#include "joystick.h"#include <stdio.h>
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
#include "user_interface.h"

//---------------------------------- MACROS -----------------------------------
#define BUZZER_PIN GPIO_NUM_26
//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------

//------------------------- STATIC DATA & CONSTANTS ---------------------------

static const char *TAG = "MAIN";
//------------------------------- GLOBAL DATA ---------------------------------

//------------------------------ PUBLIC FUNCTIONS -----------------------------
void app_main(void)
{
   

   (void) _button_init (GPIO_BUTTON_1);
   morse_init ();
   led_init(LED_BLUE);
   led_init (LED_RED);
   //buzzer_init ();
   //buzzer_start();
   init_pwm ();

   tictactoe_init();
   esp_err_t err = temp_sensor_init();
   if (err != ESP_OK) ESP_LOGI(TAG, "temp/hum sensor init failed");

   my_mqtt_init();

   joystick_init();

}

//---------------------------- PRIVATE FUNCTIONS ------------------------------

//---------------------------- INTERRUPT HANDLERS -----------------------------
