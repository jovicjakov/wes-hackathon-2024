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
#include "joystick.h"
//---------------------------------- MACROS -----------------------------------

//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------

//------------------------- STATIC DATA & CONSTANTS ---------------------------

static const char *TAG = "MAIN";
//------------------------------- GLOBAL DATA ---------------------------------

//------------------------------ PUBLIC FUNCTIONS -----------------------------
void app_main(void)
{
    // user_interface_init();
    tictactoe_init();
    esp_err_t err = temp_sensor_init();
    if (err != ESP_OK)
        ESP_LOGI(TAG, "temp/hum sensor init failed");

    my_mqtt_init();

    joystick_init();
}

//---------------------------- PRIVATE FUNCTIONS ------------------------------

//---------------------------- INTERRUPT HANDLERS -----------------------------
