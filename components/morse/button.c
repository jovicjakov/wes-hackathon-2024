#include "inc/button.h"
#include "hal/gpio_types.h"
#include <esp_log.h>
#include "esp_timer.h"

#include "../led/led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <stdio.h>


QueueHandle_t p_morse_queue = NULL;
static const char *TAG = "BUTTON";

static int64_t last_debounce_time = 0;
#define DEBOUNCE_DELAY 500000  // Debounce delay in microseconds
#define MORSE_QUEUE_SIZE (20U)
#define BUZZER_GPIO 2 // GPIO connected to the buzzer


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

    // p_morse_queue = xQueueCreate(MORSE_QUEUE_SIZE, sizeof(int));
    // if(p_morse_queue == NULL)
    // {
    //     printf("Morse queue was not initialized successfully\n");
    //     return;
    // }

    // if(pdPASS != xTaskCreate(&_morse_task, "morse_task", 2 * 1024, NULL, 5, &p_morse_queue))
    // {
    //     printf("Morse task was not initialized successfully\n");
    //     return;
    // }

    return esp_err;
}



esp_err_t buzzer_init(void)
{
    // Configuration structure for GPIO
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE, // Disable interrupt
        .mode = GPIO_MODE_OUTPUT,       // Set as output mode
        .pin_bit_mask = (1ULL << BUZZER_GPIO), // Bit mask for GPIO2
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // Disable pull-down
        .pull_up_en = GPIO_PULLUP_DISABLE,     // Disable pull-up
    };

    // Configure GPIO
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        // Handle error
        return err;
    }

    return ESP_OK; // Initialization successful
}

//------------------------------ PUBLIC FUNCTIONS -----------------------------
void morse_init(void)
{
    led_init (LED_BLUE);
    p_morse_queue = xQueueCreate(MORSE_QUEUE_SIZE, sizeof(int));
    TaskHandle_t morse_task_handle; // Declare a TaskHandle_t variable
    if(p_morse_queue == NULL)
    {
        ESP_LOGI(TAG, "Morse queue was not initialized successfully\n");
        return;
    }

    if(pdPASS != xTaskCreate(&_morse_task, "morse_task", 2 * 1024, NULL, 5, &morse_task_handle))
    {
        ESP_LOGI(TAG, "Morse task was not initialized successfully\n");
        return;
    }
}

//---------------------------- PRIVATE FUNCTIONS ------------------------------
static void send_sos_ticks (){

}

static void _morse_task(void *p_parameter) {

    int morse_event;
    
    for (;;) {
        gpio_set_level (BUZZER_GPIO, 0);
    }
}
