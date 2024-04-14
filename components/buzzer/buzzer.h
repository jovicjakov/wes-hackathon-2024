#ifndef BUZZER_H
#define BUZZER_H

#include "driver/ledc.h"
#include "esp_err.h"

// Define GPIO pin for the buzzer
#define BUZZER_GPIO 18

// Define PWM parameters
#define PWM_CHANNEL    LEDC_CHANNEL_0
#define PWM_MODE       LEDC_HIGH_SPEED_MODE
#define PWM_FREQ_HZ    1000
#define PWM_RESOLUTION LEDC_TIMER_13_BIT

#define SHORT_BEEP_DURATION 100 // Duration of short beep in milliseconds
#define LONG_BEEP_DURATION 300  // Duration of long beep in milliseconds
#define PAUSE_BETWEEN_BEEPS 100 // Pause between each beep in milliseconds
#define PAUSE_BETWEEN_LETTERS 300 // Pause between each letter in milliseconds

// Function to initialize PWM
esp_err_t init_pwm();

// Function to control the buzzer
void buzzer_control(int duty_cycle);

#endif /* BUZZER_H */
