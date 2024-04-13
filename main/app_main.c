/**
 * @file main.c
 *
 * @brief
 *
 * COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
 * All rights reserved.
 */

//--------------------------------- INCLUDES ----------------------------------
#include "user_interface.h"
#include <stdio.h>
#include "../components/morse/inc/button.h"
//#include <VMA320.h>
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

//---------------------------------- MACROS -----------------------------------
#define BUZZER_PIN GPIO_NUM_26
//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------

//------------------------- STATIC DATA & CONSTANTS ---------------------------

//------------------------------- GLOBAL DATA ---------------------------------

//------------------------------ PUBLIC FUNCTIONS -----------------------------
void app_main(void)
{
   (void) _button_init (GPIO_BUTTON_1);
   morse_init ();
   //buzzer_init ();
   //buzzer_start();
   init_pwm ();
}

//---------------------------- PRIVATE FUNCTIONS ------------------------------

//---------------------------- INTERRUPT HANDLERS -----------------------------
