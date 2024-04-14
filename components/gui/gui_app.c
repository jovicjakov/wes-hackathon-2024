/**
 * @file gui_app.c
 *
 * @brief This file runs the UI.
 *
 * COPYRIGHT NOTICE: (c) 2023 Byte Lab Grupa d.o.o.
 * All rights reserved.
 */

//--------------------------------- INCLUDES ----------------------------------
#include "gui_app.h"
// #include "tictactoe.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/portmacro.h"
#include "esp_log.h"
#include "tictactoe.h"

/* Littlevgl specific */
#include "lvgl.h"
#include "lvgl_helpers.h"
#include "lv_conf.h"
#include "my_mqtt.h"
#include "temp_hum_sensor.h"
#include "time.h"
//---------------------------------- MACROS -----------------------------------
#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH 320

#define Y_ALIGN 30
#define X_ALIGN 45
#define X_ALIGN_CENTER 15

#define FADE_IN_TIME 500

//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------
/**
 * @brief This function handles events that happen on LVGL buttons.
 *
 * @param [in] p_event Pointer to the event type.
 */
static void _button_event_handler(lv_event_t *p_event);
static void _wait_for_sensor_input_task(void *p_parameter);

/**
 * @brief The function unblockingly sends an event to the user interface queue.
 *
 * @param event Gui event to be sent.
 */

static void button_matrix_init(void);
static void labels_init(void);
static void select_first_player_buttons_init(void);
static void sensor_table_init(void);

//------------------------- STATIC DATA & CONSTANTS ---------------------------

static const char *TAG = "GUI_APP";
static const char *btnm_map[] = {" ", " ", " ", "\n", " ",
                                 " ", " ", "\n", " ", " ", " ", ""};
static struct tm timeinfo;

//------------------------------- GLOBAL DATA ---------------------------------
// extern QueueHandle_t p_user_interface_queue;
lv_obj_t **p_labels;
lv_obj_t *btnm1;
lv_obj_t *p_btn_me_first;
lv_obj_t *p_btn_earthling_first;
lv_obj_t *mqtt_connected_label;
lv_obj_t *temp_label;
lv_obj_t *humidity_label;
lv_obj_t *time_label;
lv_obj_t *table;
lv_obj_t *game_end_label;

lv_obj_t *screen1;
lv_obj_t *screen2;
lv_obj_t *screen3;
lv_obj_t *screen4;

QueueHandle_t gui_queue = NULL;
QueueHandle_t reset_queue = NULL;
QueueHandle_t temp_hum_to_gui_queue = NULL;
extern QueueHandle_t timeQueue;
QueueHandle_t joystick_to_gui_queue = NULL;

//------------------------------ PUBLIC FUNCTIONS -----------------------------

void gui_app_init(void)
{
    screen1 = lv_obj_create(NULL);
    screen2 = lv_obj_create(NULL);
    screen3 = lv_obj_create(NULL);
    screen4 = lv_obj_create(NULL);
    table = lv_table_create(screen3);
    lv_obj_align(screen4, LV_ALIGN_CENTER, 0, 0);
    button_matrix_init();
    select_first_player_buttons_init();
    labels_init();
    sensor_table_init();

    gui_queue = xQueueCreate(GUI_QUEUE_SIZE, sizeof(gui_app_event_t));
    reset_queue = xQueueCreate(GUI_QUEUE_SIZE, sizeof(tictactoe_gamestate_t));
    temp_hum_to_gui_queue = xQueueCreate(GUI_QUEUE_SIZE, sizeof(gui_sensor_packet_t));
    joystick_to_gui_queue = xQueueCreate(GUI_QUEUE_SIZE, sizeof(gui_sensor_packet_t));
    if (gui_queue == NULL)
    {
        printf("User interface queue was not initialized successfully\n");
        return;
    }

    while (!is_mqtt_connected())
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    lv_scr_load(screen2);
    if (pdPASS != xTaskCreate(&_wait_for_sensor_input_task, "_wait_for_sensor_input_task", 2 * 1024, NULL, 5, NULL))
    {
        printf("_wait_for_sensor_input_tasktask was not initialized successfully\n");
    }
    // lv_label_set_text(mqtt_connected_label, "Who do you want to play first Uranusborn?");
}

void crtaj_xo(int position, char *symbol)
{
    lv_label_set_text(p_labels[position], symbol);
}

//---------------------------- PRIVATE FUNCTIONS ------------------------------

static void matrix_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        uint32_t id = lv_btnmatrix_get_selected_btn(obj);
        const char *txt = lv_btnmatrix_get_btn_text(obj, id);
        gui_app_event_t event = id;
        ESP_LOGI(TAG, "%lu was pressed\n", id);
        xQueueSend(gui_queue, &event, 0U);
    }
}

static void button_matrix_init(void)
{
    btnm1 = lv_btnmatrix_create(screen1);
    lv_btnmatrix_set_map(btnm1, btnm_map);
    lv_obj_align(btnm1, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btnm1, matrix_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_set_width(btnm1, SCREEN_WIDTH);
    lv_obj_set_height(btnm1, SCREEN_HEIGHT);
}

static void labels_init()
{
    game_end_label = lv_label_create(screen4);
    lv_obj_set_style_text_font(game_end_label, &lv_font_montserrat_14, 0);
    lv_label_set_text(game_end_label, " ");
    lv_obj_align_to(game_end_label, NULL, LV_ALIGN_CENTER, 0, 0);

    mqtt_connected_label = lv_label_create(screen2);
    lv_obj_set_style_text_font(mqtt_connected_label, &lv_font_montserrat_14, 0);
    lv_label_set_text(mqtt_connected_label, "Who do you want to play first Uranusborn?");
    lv_obj_align_to(mqtt_connected_label, NULL, LV_ALIGN_TOP_MID, 0, 2 * Y_ALIGN);

    p_labels = lv_mem_alloc(9 * sizeof(btnm1));
    for (int i = 0; i < 9; i++)
    {
        p_labels[i] = lv_label_create(screen1);
        lv_label_set_text(p_labels[i], " ");
        lv_obj_set_style_text_font(p_labels[i], &lv_font_montserrat_38, 0);
    }
    lv_obj_align_to(p_labels[0], NULL, LV_ALIGN_TOP_LEFT, X_ALIGN, Y_ALIGN);
    lv_obj_align_to(p_labels[1], NULL, LV_ALIGN_TOP_MID, 0, Y_ALIGN);
    lv_obj_align_to(p_labels[2], NULL, LV_ALIGN_TOP_RIGHT, -X_ALIGN, Y_ALIGN);
    lv_obj_align_to(p_labels[3], NULL, LV_ALIGN_LEFT_MID, X_ALIGN, 0);
    lv_obj_align_to(p_labels[4], NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align_to(p_labels[5], NULL, LV_ALIGN_RIGHT_MID, -X_ALIGN, 0);
    lv_obj_align_to(p_labels[6], NULL, LV_ALIGN_BOTTOM_LEFT, X_ALIGN, -Y_ALIGN);
    lv_obj_align_to(p_labels[7], NULL, LV_ALIGN_BOTTOM_MID, 0, -Y_ALIGN);
    lv_obj_align_to(p_labels[8], NULL, LV_ALIGN_BOTTOM_RIGHT, -X_ALIGN, -Y_ALIGN);
}

static void select_first_player_buttons_init(void)
{

    /* Create buttons */
    lv_obj_t *p_label_me_first;
    p_btn_me_first = lv_btn_create(screen2);
    lv_obj_align_to(p_btn_me_first, NULL, LV_ALIGN_CENTER, -7 * X_ALIGN_CENTER, Y_ALIGN / 2);
    p_label_me_first = lv_label_create(p_btn_me_first);
    lv_label_set_text(p_label_me_first, "Me first");

    lv_obj_t *p_label_earthling_first;
    p_btn_earthling_first = lv_btn_create(screen2);
    lv_obj_align_to(p_btn_earthling_first, NULL, LV_ALIGN_CENTER, X_ALIGN_CENTER, Y_ALIGN / 2);
    p_label_earthling_first = lv_label_create(p_btn_earthling_first);
    lv_label_set_text(p_label_earthling_first, "Earthling first");

    // Add buttons callback
    (void)lv_obj_add_event_cb(p_btn_me_first, _button_event_handler, LV_EVENT_CLICKED, NULL);
    (void)lv_obj_add_event_cb(p_btn_earthling_first, _button_event_handler, LV_EVENT_CLICKED, NULL);
}

static void sensor_table_init()
{
    lv_table_set_cell_value(table, 0, 0, "Date and Time");
    lv_table_set_cell_value(table, 1, 0, "Temperature");
    lv_table_set_cell_value(table, 2, 0, "Humidity");
    lv_table_set_cell_value(table, 0, 1, " ");
    lv_table_set_cell_value(table, 1, 1, " ");
    lv_table_set_cell_value(table, 2, 1, " ");
}

static void _button_event_handler(lv_event_t *p_event)
{

    if (p_btn_me_first == p_event->target && is_mqtt_connected())
    {
        if (LV_EVENT_CLICKED == p_event->code)
        {
            printf("me first\n");
            gui_app_event_t event = GUI_APP_EVENT_ME_FIRST_BUTTON_PRESSED;
            if (gui_queue != NULL)
            {
                printf("gui: sent to tictactoe\n");
                xQueueSend(gui_queue, &event, 0U);
            }
            lv_scr_load_anim(screen1, LV_SCR_LOAD_ANIM_FADE_IN, FADE_IN_TIME, 0, false);
        }
    }
    else if (p_btn_earthling_first == p_event->target && is_mqtt_connected())
    {
        if (LV_EVENT_CLICKED == p_event->code)
        {
            printf("earthling first\n");
            gui_app_event_t event = GUI_APP_EVENT_EARTHLING_FIRST_BUTTON_PRESSED;
            if (gui_queue != NULL)
            {
                printf("gui: sent to tictactoe\n");
                xQueueSend(gui_queue, &event, 0U);
            }
            lv_scr_load_anim(screen1, LV_SCR_LOAD_ANIM_FADE_IN, FADE_IN_TIME, 0, false);
        }
    }
    else
    {
        /* Unknown button event. */
    }
}

static void _wait_for_sensor_input_task(void *p_parameter)
{
    for (;;)
    {

        int switch_screen = 0;
        TempHumData packet;
        tictactoe_gamestate_t end;
        if (temp_hum_to_gui_queue != NULL && (xQueueReceive(temp_hum_to_gui_queue, &packet, 100 / portTICK_PERIOD_MS) == pdTRUE))
        {
            char temp[8];
            char temp_tmp[10];
            snprintf(temp, sizeof(temp) + 1, "%f", packet.temperature + 100);
            sprintf(temp_tmp, "%c%c%c%c%c °C", temp[1], temp[2], temp[3], temp[4], temp[5]);
            char hum[8];
            snprintf(hum, 8 + 1, "%f", packet.humidity);
            // ESP_LOGI(TAG, "~~~~~ RECIEVED TEMP [%c%c%c%c%c%c%c%c]", temp[0], temp[1], temp[2], temp[3], temp[4], temp[5], temp[6], temp[7]);
            lv_table_set_cell_value(table, 1, 1, temp_tmp);
            lv_table_set_cell_value(table, 2, 1, hum);
        }
        if (timeQueue != NULL && (xQueueReceive(timeQueue, &timeinfo, 100 / portTICK_PERIOD_MS) == pdTRUE))
        {

            char dateTime[30]; // Increase buffer size to hold the full date and time
            // Format the time string to include the date as well (e.g., "YYYY-MM-DD HH:MM")
            strftime(dateTime, sizeof(dateTime), "%Y-%m-%d %H:%M", &timeinfo);
            lv_table_set_cell_value(table, 0, 1, dateTime);
        }
        if (joystick_to_gui_queue != NULL && (xQueueReceive(joystick_to_gui_queue, &switch_screen, 200 / portTICK_PERIOD_MS) == pdTRUE))
        {
            if (switch_screen == 1 && (lv_disp_get_scr_act(NULL) == screen1))
            {
                lv_scr_load_anim(screen3, LV_SCR_LOAD_ANIM_MOVE_LEFT, 2 * FADE_IN_TIME, 0, false);
            }
            else if (switch_screen == -1 && (lv_disp_get_scr_act(NULL) == screen3))
            {
                lv_scr_load_anim(screen1, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 2 * FADE_IN_TIME, 0, false);
            }
        }
        if (reset_queue != NULL && (xQueueReceive(reset_queue, &end, 100 / portTICK_PERIOD_MS) == pdTRUE))
        {

            switch (end)
            {
            case WIN:
                lv_label_set_text(game_end_label, "POWER TO URANUS!");
                break;
            case LOSS:
                lv_label_set_text(game_end_label, "Earthlings won...");
                break;
            case DRAW:
                lv_label_set_text(game_end_label, "Draw... I'll get you next time");
                break;
            default:
                break;
            }
            lv_scr_load_anim(screen4, LV_SCR_LOAD_ANIM_FADE_IN, 3 * FADE_IN_TIME, 0, false);
            for (int i = 0; i < 9; i++)
            {
                lv_label_set_text(p_labels[i], " ");
            }

            lv_scr_load_anim(screen2, LV_SCR_LOAD_ANIM_FADE_IN, 2 * FADE_IN_TIME, 5000 / portTICK_PERIOD_MS, false);
        }
    }
}

//---------------------------- INTERRUPT HANDLERS -----------------------------
