//--------------------------------- INCLUDES ----------------------------------
#include "driver/gpio.h"
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

//---------------------------------- MACROS -----------------------------------
#define GPIO_BIT_MASK(X)    ((1ULL << (X)))
#define GPIO_BUTTON_1 (36U)

//-------------------------------- DATA TYPES ---------------------------------
/**
 * @brief Pointer to the function that will be called on button press.
 *
 * @param [in] p_arg Pointer to the users custom argument.
 *
 */
//typedef void (*button_pressed_isr_t)(void *p_arg);

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------
/**
 * @brief Callback function for button 1 press.
 *
 * @param [in] p_arg Pointer to the pin that caused the callback.
 */
//static void IRAM_ATTR _button_isr(void *p_arg);

/**
 * @brief The function initializes button and its interrupt service routine.
 *
 * @param pin Pin to which the button is connected to.
 * @param p_isr Pointer to the ISR that will be executed on button press.
 * @return esp_err_t ESP_OK on success, fail otherwise.
 */
esp_err_t _button_init(uint8_t pin);

void morse_init(void);
static void _morse_task(void *p_parameter);
esp_err_t buzzer_init(void);