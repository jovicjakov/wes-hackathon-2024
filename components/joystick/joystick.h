#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stddef.h>

esp_err_t joystick_init(void);
void joystick_startReadingStates(void);

#endif