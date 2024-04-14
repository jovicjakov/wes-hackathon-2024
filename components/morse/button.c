#include "inc/button.h"
#include "esp_err.h"
#include "hal/gpio_types.h"
#include <esp_log.h>
#include "esp_timer.h"

#include "../led/led.h"
#include "../buzzer/buzzer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <stdio.h>


QueueHandle_t p_morse_queue = NULL;
QueueHandle_t p_led_queue = NULL;
static const char *TAG = "BUTTON";

static int64_t last_debounce_time = 0;
#define DEBOUNCE_DELAY 500000  // Debounce delay in microseconds
#define MORSE_QUEUE_SIZE (20U)
#define LED_QUEUE_SIZE (20U)
#define BUZZER_GPIO 2 // GPIO connected to the buzzer
#define BUZZER_PWM_CHANNEL LEDC_CHANNEL_0 // PWM channel for the buzzer
#define BUZZER_PWM_FREQ_HZ 1000 // PWM frequency in Hz
#define BUZZER_PWM_RESOLUTION LEDC_TIMER_13_BIT // PWM resolution (13-bit)


//---------------------------- INTERRUPT HANDLERS ------------------------------
static void IRAM_ATTR _button_isr(void *p_arg) 
{
    (void)p_arg; /* Suppress the unused variable warning*/

    //software debouncing
    int64_t current_time = esp_timer_get_time();
    if (current_time - last_debounce_time > DEBOUNCE_DELAY) {
        last_debounce_time = current_time;
        int signal = 1;
        xQueueSendFromISR(p_morse_queue, &signal, NULL);
        xQueueSendFromISR(p_led_queue, &signal, NULL);
    } 
}

esp_err_t _button_init(uint8_t pin) 
{
    led_init (LED_BLUE);
    // Configure the GPIO.
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = (GPIO_INTR_POSEDGE),
    };

    esp_err_t esp_err = gpio_config(&io_conf);

    if(ESP_OK == esp_err)
    {
        // Change gpio interrupt type for a pin.
        esp_err = gpio_set_intr_type(io_conf.pin_bit_mask, io_conf.intr_type);
    }

    if(ESP_OK == esp_err)
    {
        // Install gpio isr service.
        gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
        /* esp_err is not assigned to it because it returns invalid
        statuses when called many times (for many buttons)*/
    }

    if(ESP_OK == esp_err)
    {
        // Hook isr handler for specific gpio pin.
        esp_err = gpio_isr_handler_add(pin, _button_isr, (void *)&pin);
    }
    return esp_err;
}

//------------------------------ PUBLIC FUNCTIONS -----------------------------
esp_err_t morse_init(void)
{
    p_morse_queue = xQueueCreate(MORSE_QUEUE_SIZE, sizeof(int));
    p_led_queue = xQueueCreate (LED_QUEUE_SIZE, sizeof (int));
    TaskHandle_t morse_task_handle; // Declare a TaskHandle_t variable
    TaskHandle_t led_task_handle; // Declare a TaskHandle_t variable
    if(p_morse_queue == NULL && p_led_queue == NULL)
    {
        ESP_LOGI(TAG, "Morse queue was not initialized successfully\n");
        return ESP_FAIL;
    }

    if(pdPASS != xTaskCreate(&_morse_task, "morse_task", 2 * 1024, NULL, 5, &morse_task_handle))
    {
        ESP_LOGI(TAG, "Morse task was not initialized successfully\n");
        return ESP_FAIL;
    }
    
    if (pdPASS != xTaskCreate(&_led_task, "led_task", 2 * 1024, NULL, 5, &led_task_handle)) {
        ESP_LOGI(TAG, "Led task was not initialized successfully\n");
        return ESP_FAIL;
    }

    return ESP_OK;
}

//---------------------------- PRIVATE FUNCTIONS ------------------------------
static void send_sos_ticks (){
    const char *sos = "... --- ...";

    for (const char *ch = sos; *ch != '\0'; ch++) {
        int signal = 1;
        if (*ch == '.') { // Short beep
            // Turn on buzzer
            // Implement code to turn on the buzzer
            buzzer_control(8000);

            // Delay for short beep duration
            vTaskDelay(pdMS_TO_TICKS(SHORT_BEEP_DURATION));

            // Turn off buzzer
            // Implement code to turn off the buzzer
            buzzer_control(0);
            xQueueSend (p_led_queue, &signal, NULL);

            // Pause between beeps
            vTaskDelay(pdMS_TO_TICKS(PAUSE_BETWEEN_BEEPS));
        } else if (*ch == '-') { // Long beep
            // Turn on buzzer
            // Implement code to turn on the buzzer
            buzzer_control(8000);

            // Delay for long beep duration
            vTaskDelay(pdMS_TO_TICKS(LONG_BEEP_DURATION));

            // Turn off buzzer
            // Implement code to turn off the buzzer
            buzzer_control(0);
            //xQueueSend (p_led_queue, &signal, NULL);

            // Pause between beeps
            vTaskDelay(pdMS_TO_TICKS(PAUSE_BETWEEN_BEEPS));
        } else if (*ch == ' ') { // Pause between letters
            // Pause between letters
            vTaskDelay(pdMS_TO_TICKS(PAUSE_BETWEEN_LETTERS));
        }
    }
}
static void sos_signalisation () {
    printf("LED RED");
    led_on (LED_BLUE);
    vTaskDelay(pdMS_TO_TICKS(800));
    led_off (LED_BLUE);
    vTaskDelay(pdMS_TO_TICKS(200));
}

static void _morse_task(void *p_parameter) {

    int morse_event;
    
    for (;;) {
        if((xQueueReceive(p_morse_queue, &morse_event, portMAX_DELAY) == pdTRUE)){
            printf("Morse event received %d\n", morse_event);
            //led_on (LED_BLUE);
            //buzzer_start ();
            //sos_signalisation ();
            send_sos_ticks ();
        }else {
            printf("Morse failed\n");
        }
    }
}

static void _led_task(void *p_parameter) {

    int led_event;
    
    for (;;) {
        if((xQueueReceive(p_led_queue, &led_event, portMAX_DELAY) == pdTRUE)){
            printf("Led event received %d\n", led_event);
            sos_signalisation();
        }else {
            printf("LED failed\n");
        }
    }
}
