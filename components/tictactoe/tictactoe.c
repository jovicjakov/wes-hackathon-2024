/**
 * @file user_interface.c
 *
 * @brief This file is responsible for complete user interface (LEDs, buttons
 * and GUI).
 *
 * COPYRIGHT NOTICE: (c) 2024 Byte Lab Grupa d.o.o.
 * All rights reserved.
 */

//--------------------------------- INCLUDES ----------------------------------

#include "freertos/portmacro.h"
#include "gui_app.h"
#include "gui.h"
#include "tictactoe.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <stdio.h>
#include <string.h>

//---------------------------------- MACROS -----------------------------------

//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------
/**
 * @brief tictactoe task.
 *
 * @param [in] p_parameter This is the parameter that is passed to the task.
 */
static void _tictactoe_task(void *p_parameter);
static void tictactoe_first_move_task(void *p_parameter);

//------------------------- STATIC DATA & CONSTANTS ---------------------------
static TaskHandle_t p_tictactoe_task = NULL;

//------------------------------- GLOBAL DATA ---------------------------------
QueueHandle_t p_tictactoe_queue_send = NULL;
QueueHandle_t p_tictactoe_queue_rec = NULL;
tictactoe_turn_t playerX = DEVICE;
extern QueueHandle_t gui_queue;
tictactoe_handler_t game;

//------------------------------ PUBLIC FUNCTIONS -----------------------------
void tictactoe_init(void)
{

   gui_init();
   p_tictactoe_queue_send = xQueueCreate(TICTACTOE_QUEUE_SIZE, sizeof(tictactoe_handler_t));
   p_tictactoe_queue_rec = xQueueCreate(TICTACTOE_QUEUE_SIZE, sizeof(tictactoe_handler_t));
   if (p_tictactoe_queue_send == NULL)
   {
      printf("User interface queue was not initialized successfully\n");
      return;
   }
   memset(&game, 0, sizeof(game));
   TaskHandle_t p_tictactoe_first_move_task = NULL;
   if (pdPASS != xTaskCreate(&tictactoe_first_move_task, "tictactoe_first_move_task", 2 * 1024, NULL, 5, &p_tictactoe_first_move_task))
   {
      printf("tictactoe_first_move_task was not initialized successfully\n");
      return;
   }
}

static void tictactoe_first_move_task()
{
   gui_app_event_t gui_event;
   for (;;)
   {
      if ((gui_queue != NULL) && (xQueueReceive(gui_queue, &gui_event, portMAX_DELAY) == pdTRUE))
      {
         printf("ttt: recieved bttn press\n");
         if (gui_event == GUI_APP_EVENT_ME_FIRST_BUTTON_PRESSED)
         {
            playerX = DEVICE;
            game.index_of_O[1] = 1;
            game.turn = playerX;
         }
         else if (gui_event == GUI_APP_EVENT_ME_FIRST_BUTTON_PRESSED)
         {
            playerX = SERVER;
            game.turn = playerX;
         }
         if (p_tictactoe_queue_send != NULL)
         {
            printf("ttt: sending struct to mqqt task\n");
            xQueueSend(p_tictactoe_queue_send, &game, 0U);
         }
         if (pdPASS != xTaskCreate(&_tictactoe_task, "tictactoe_task", 2 * 1024, NULL, 5, &p_tictactoe_task))
         {
            printf("User interface task was not initialized successfully\n");
         }

      }
   }
}

//---------------------------- PRIVATE FUNCTIONS ------------------------------
static void _tictactoe_task(void *p_parameter)
{
   for (;;)
   {
      tictactoe_handler_t tictactoe_event;
      /* Blockingly wait on an event. */
      if ((p_tictactoe_queue_rec != NULL) && (xQueueReceive(p_tictactoe_queue_rec, &tictactoe_event, portMAX_DELAY) == pdTRUE))
      {
         printf("MQTT event received \n");

      }
   }
}

//---------------------------- INTERRUPT HANDLERS -----------------------------