/**
 * @file gui_app.c
 *
 * @brief See the source file.
 *
 * COPYRIGHT NOTICE: (c) 2023 Byte Lab Grupa d.o.o.
 * All rights reserved.
 */

#ifndef __GUI_APP_C__
#define __GUI_APP_C__

#ifdef __cplusplus
extern "C"
{
#endif

//--------------------------------- INCLUDES ----------------------------------

//---------------------------------- MACROS -----------------------------------
#define GUI_QUEUE_SIZE (20U)

    //-------------------------------- DATA TYPES ---------------------------------
    /**
     * @brief Enums hold gui app events.
     *
     */
    typedef enum
    {
        GUI_APP_EVENT_MATRIX_0_PRESSED,
        GUI_APP_EVENT_MATRIX_1_PRESSED,
        GUI_APP_EVENT_MATRIX_2_PRESSED,
        GUI_APP_EVENT_MATRIX_3_PRESSED,
        GUI_APP_EVENT_MATRIX_4_PRESSED,
        GUI_APP_EVENT_MATRIX_5_PRESSED,
        GUI_APP_EVENT_MATRIX_6_PRESSED,
        GUI_APP_EVENT_MATRIX_7_PRESSED,
        GUI_APP_EVENT_MATRIX_8_PRESSED,
        GUI_APP_EVENT_ME_FIRST_BUTTON_PRESSED,
        GUI_APP_EVENT_EARTHLING_FIRST_BUTTON_PRESSED

    } gui_app_event_t;

    typedef struct
    {
        int hours;
        int minutes;
        int seconds;
        int temperature;
        int humidity;
    } gui_sensor_packet_t;

    //---------------------- PUBLIC FUNCTION PROTOTYPES --------------------------
    /**
     * @brief The function initialized first screen.
     *
     */
    void gui_app_init(void);
    void crtaj_xo(int position, char *symbol);

#ifdef __cplusplus
}
#endif

#endif // __GUI_APP_C__
