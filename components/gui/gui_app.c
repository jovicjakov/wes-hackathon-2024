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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

/* Littlevgl specific */
#include "lvgl.h"
#include "lvgl_helpers.h"
#include "lv_conf.h"
//---------------------------------- MACROS -----------------------------------
#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH 320

#define Y_ALIGN 30
#define X_ALIGN 45
#define X_ALIGN_CENTER 30

//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------
/**
 * @brief This function handles events that happen on LVGL buttons.
 *
 * @param [in] p_event Pointer to the event type.
 */
static void _button_event_handler(lv_event_t *p_event);

/**
 * @brief The function unblockingly sends an event to the user interface queue.
 *
 * @param event Gui event to be sent.
 */

static void button_matrix_init(void);
static void select_first_player_buttons_init(void);

//------------------------- STATIC DATA & CONSTANTS ---------------------------

static const char *TAG = "GUI_APP";
static const char *btnm_map[] = {" ", " ", " ", "\n", " ",
                                 " ", " ", "\n", " ", " ", " ", ""};

//------------------------------- GLOBAL DATA ---------------------------------
// extern QueueHandle_t p_user_interface_queue;
lv_obj_t **p_labels;
lv_obj_t *btnm1;
lv_obj_t *p_btn_me_first;
lv_obj_t *p_btn_earthling_first;
QueueHandle_t gui_queue = NULL;
lv_obj_t *screen1;
lv_obj_t *screen2;

//------------------------------ PUBLIC FUNCTIONS -----------------------------

void gui_app_init(void)
{
    screen1 = lv_obj_create(NULL);
    screen2 = lv_obj_create(NULL);
    button_matrix_init();
    select_first_player_buttons_init();

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

    gui_queue = xQueueCreate(GUI_QUEUE_SIZE, sizeof(gui_app_event_t));
    if (gui_queue == NULL)
    {
        printf("User interface queue was not initialized successfully\n");
        return;
    }
    lv_scr_load(screen2);

    // crtaj_xo(1, "x");
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
    // crtaj_xo(2, "o");
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
    // crtaj_xo(8, "x");
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
    // crtaj_xo(5, "o");
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
        ESP_LOGI(TAG, "%lu was pressed\n", id);
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

static void select_first_player_buttons_init(void)
{

    /* Create buttons */
    lv_obj_t *p_label_me_first;
    p_btn_me_first = lv_btn_create(screen2);
    lv_obj_align_to(p_btn_me_first, NULL, LV_ALIGN_CENTER, -4 * X_ALIGN_CENTER, 0);
    p_label_me_first = lv_label_create(p_btn_me_first);
    lv_label_set_text(p_label_me_first, "Me first");

    lv_obj_t *p_label_earthling_first;
    p_btn_earthling_first = lv_btn_create(screen2);
    lv_obj_align_to(p_btn_earthling_first, NULL, LV_ALIGN_CENTER, X_ALIGN_CENTER, 0);
    p_label_earthling_first = lv_label_create(p_btn_earthling_first);
    lv_label_set_text(p_label_earthling_first, "Earthling first");

    // Add buttons callback
    (void)lv_obj_add_event_cb(p_btn_me_first, _button_event_handler, LV_EVENT_CLICKED, NULL);
    (void)lv_obj_add_event_cb(p_btn_earthling_first, _button_event_handler, LV_EVENT_CLICKED, NULL);
}

static void _button_event_handler(lv_event_t *p_event)
{
    if (p_btn_me_first == p_event->target)
    {
        if (LV_EVENT_CLICKED == p_event->code)
        {
            printf("me first\n");
            gui_app_event_t event = GUI_APP_EVENT_ME_FIRST_BUTTON_PRESSED;
            if (gui_queue != NULL)
                xQueueSend(gui_queue, &event, 0U);
            lv_scr_load(screen1);
        }
    }
    else if (p_btn_earthling_first == p_event->target)
    {
        if (LV_EVENT_CLICKED == p_event->code)
        {
            printf("earthling first\n");
            gui_app_event_t event = GUI_APP_EVENT_EARTHLING_FIRST_BUTTON_PRESSED;
            if (gui_queue != NULL)
                xQueueSend(gui_queue, &event, 0U);
            lv_scr_load(screen1);
        }
    }
    else
    {
        /* Unknown button event. */
    }
}

//---------------------------- INTERRUPT HANDLERS -----------------------------
