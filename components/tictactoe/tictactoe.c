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
#include "esp_log.h"

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
int check_winner(const int *indexes);
int check_game_state(tictactoe_handler_t *game);
void refresh_game_state();

//------------------------- STATIC DATA & CONSTANTS ---------------------------
static TaskHandle_t p_tictactoe_task = NULL;
static const char *TAG = "tictactoe";
const int winning_combinations[8][3] = {
    {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, // Horizontal
    {0, 3, 6},
    {1, 4, 7},
    {2, 5, 8}, // Vertical
    {0, 4, 8},
    {2, 4, 6} // Diagonal
};

//------------------------------- GLOBAL DATA ---------------------------------
QueueHandle_t p_tictactoe_queue_send = NULL;
QueueHandle_t p_tictactoe_queue_rec = NULL;
tictactoe_turn_t playerX = DEVICE;
extern QueueHandle_t gui_queue;
tictactoe_handler_t game;
tictactoe_gamestate_t gamestate = IN_PROGRESS;

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
         printf("ttt: recieved bttn press, %d\n", gui_event);

         if (gui_event == GUI_APP_EVENT_ME_FIRST_BUTTON_PRESSED)
         {
            playerX = SERVER;
            game.index_of_O[1] = 1;
            game.turn = SERVER;
            crtaj_xo(1, "o");
         }
         else if (gui_event == GUI_APP_EVENT_EARTHLING_FIRST_BUTTON_PRESSED)
         {
            playerX = SERVER;
            game.turn = SERVER;
         }
         else
         {
            ESP_LOGI(TAG, "PLAYERX = %d", playerX);
            if (playerX == DEVICE && game.index_of_X[gui_event] == 0 && game.turn == DEVICE)
            {
               game.index_of_X[gui_event] = 1;
               crtaj_xo(gui_event, "x");
               game.turn = SERVER;
               xQueueSend(p_tictactoe_queue_send, &game, 0U);
            }
            else if (playerX == SERVER && game.index_of_O[gui_event] == 0 && game.turn == DEVICE)
            {
               game.index_of_O[gui_event] = 1;
               crtaj_xo(gui_event, "o");
               game.turn = SERVER;
               xQueueSend(p_tictactoe_queue_send, &game, 0U);
            }
            refresh_game_state();
            continue;
         }
         if (p_tictactoe_queue_send != NULL)
         {
            printf("ttt: sending struct to mqqt task: turn = %d\n", game.turn);
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

// PRIMAM POTEZ OD SERVERA
static void _tictactoe_task(void *p_parameter)
{
   for (;;)
   {
      tictactoe_handler_t tictactoe_event;
      /* Blockingly wait on an event. */
      if ((p_tictactoe_queue_rec != NULL) && (xQueueReceive(p_tictactoe_queue_rec, &tictactoe_event, portMAX_DELAY) == pdTRUE))
      {
         ESP_LOGI(TAG, "MQTT event received");
         game.turn = DEVICE;
         for (int i = 0; i < 9; i++)
         {
            printf("%d ", tictactoe_event.index_of_X[i]);
            if (tictactoe_event.index_of_X[i] != 0 && game.index_of_X[i] == 0 && game.index_of_O[i] == 0)
            {
               game.index_of_X[i] = 1;

               crtaj_xo(i, "x");
            }
         }
         printf("\n");
         for (int i = 0; i < 9; i++)
         {
            printf("%d ", tictactoe_event.index_of_O[i]);
            if (tictactoe_event.index_of_O[i] != 0 && game.index_of_O[i] == 0 && game.index_of_O[i] == 0)
            {
               game.index_of_O[i] = 1;
               crtaj_xo(i, "o");
            }
         }
         refresh_game_state();
      }
   }
}

// Function to check if a player has won
int check_winner(const int *indexes)
{
   for (int i = 0; i < 8; ++i)
   {
      int a = winning_combinations[i][0];
      int b = winning_combinations[i][1];
      int c = winning_combinations[i][2];
      if (indexes[a] && indexes[b] && indexes[c])
      {
         return 1; // Player X wins
      }
   }
   return 0; // No winner yet
}

// Function to check game state
int check_game_state(tictactoe_handler_t *game)
{
   if (check_winner(game->index_of_X))
   {
      return playerX == DEVICE ? WIN : LOSS; // Player X wins
   }
   else if (check_winner(game->index_of_O))
   {
      return playerX == DEVICE ? LOSS : WIN; // Player O wins
   }

   // Check for draw
   int is_draw = 1;
   for (int i = 0; i < 9; ++i)
   {
      if (!game->index_of_X[i] && !game->index_of_O[i])
      {
         is_draw = 0; // There are still empty cells
         break;
      }
   }
   if (is_draw)
   {
      return DRAW; // Draw
   }

   // Game in progress
   return IN_PROGRESS;
}

void refresh_game_state()
{
   int game_state = check_game_state(&game);
   switch (game_state)
   {
   case WIN:
      ESP_LOGW(TAG, "WE WON");
      break;
   case LOSS:
      ESP_LOGW(TAG, "THE EARTHLINGS WON...");
      break;
   case DRAW:
      ESP_LOGW(TAG, "ILL GET YOU NEXT TIME");
      break;
   default:
      break;
   }
}

//---------------------------- INTERRUPT HANDLERS -----------------------------