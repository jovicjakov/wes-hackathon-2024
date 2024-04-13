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

//------------------------- STATIC DATA & CONSTANTS ---------------------------
static TaskHandle_t p_tictactoe_task = NULL;

//------------------------------- GLOBAL DATA ---------------------------------
QueueHandle_t p_tictactoe_queue = NULL;
tictactoe_turn_t playerX = DEVICE;
extern QueueHandle_t gui_queue;
tictactoe_handler_t game;

//------------------------------ PUBLIC FUNCTIONS -----------------------------
void tictactoe_init(void)
{

   gui_init();
   p_tictactoe_queue = xQueueCreate(TICTACTOE_QUEUE_SIZE, sizeof(tictactoe_handler_t));
   if (p_tictactoe_queue == NULL)
   {
      printf("User interface queue was not initialized successfully\n");
      return;
   }
   memset(&game, 0, sizeof(game));
}

void tictactoe_first_move()
{
   gui_app_event_t gui_event;
   if ((gui_queue != NULL) && (xQueueReceive(gui_queue, &gui_event, 0U) == pdTRUE))
   {
      if (gui_event == GUI_APP_EVENT_ME_FIRST_BUTTON_PRESSED)
      {
         playerX = DEVICE;
         game.index_of_X[1] = 1;
         game.turn = playerX;
      }
      else if (gui_event == GUI_APP_EVENT_ME_FIRST_BUTTON_PRESSED)
      {
         playerX = SERVER;
         game.turn = playerX;
      }
      if (p_tictactoe_queue != NULL)
         xQueueSend(p_tictactoe_queue, &game, 0U);
   }

   if (pdPASS != xTaskCreate(&_tictactoe_task, "tictactoe_task", 2 * 1024, NULL, 5, &p_tictactoe_task))
   {
      printf("User interface task was not initialized successfully\n");
      return;
   }
}

//---------------------------- PRIVATE FUNCTIONS ------------------------------
static void _tictactoe_task(void *p_parameter)
{
   for (;;)
   {
      tictactoe_handler_t tictactoe_event;
      /* Blockingly wait on an event. */
      if ((p_tictactoe_queue != NULL) && (xQueueReceive(p_tictactoe_queue, &tictactoe_event, portMAX_DELAY) == pdTRUE))
      {
         printf("GUI event received \n");
      }
   }
}

//---------------------------- INTERRUPT HANDLERS -----------------------------