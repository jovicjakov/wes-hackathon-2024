#include <stdio.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"

// Define GPIO pin for the buzzer
#define BUZZER_GPIO 2

// Define PWM parameters
#define PWM_CHANNEL    LEDC_CHANNEL_0
#define PWM_MODE       LEDC_HIGH_SPEED_MODE
#define PWM_FREQ_HZ    1000
#define PWM_RESOLUTION LEDC_TIMER_13_BIT

// Function to initialize PWM
esp_err_t init_pwm() {
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz = PWM_FREQ_HZ,
        .speed_mode = PWM_MODE,
        .timer_num = LEDC_TIMER_0
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .channel    = PWM_CHANNEL,
        .duty       = 0,
        .gpio_num   = BUZZER_GPIO,
        .speed_mode = PWM_MODE,
        .timer_sel  = LEDC_TIMER_0
    };
    esp_err_t ret = ledc_channel_config(&ledc_channel);

    return ret;
}

// Function to control the buzzer
void buzzer_control(int duty_cycle) {
    ledc_set_duty(PWM_MODE, PWM_CHANNEL, duty_cycle);
    ledc_update_duty(PWM_MODE, PWM_CHANNEL);
}

// Task to demonstrate buzzer control
// void buzzer_task(void *pvParameter) {
//     while(1) {
//         // Turn on buzzer
//         buzzer_control(5000); // Adjust duty cycle as needed
//         vTaskDelay(1000 / portTICK_PERIOD_MS);

//         // Turn off buzzer
//         buzzer_control(0);
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }
