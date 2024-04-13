/**
* @file mqtt.h

* @brief See the source file.
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

#ifndef __MQTT_H__
#define __MQTT_H__

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------- INCLUDES ----------------------------------

//---------------------------------- MACROS -----------------------------------

//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PUBLIC FUNCTION PROTOTYPES --------------------------

/**
 * @brief Initializes drivers and input drivers and starts task needed for MQTT operation.
 *
 */
void my_mqtt_init(void);

#ifdef __cplusplus
}
#endif

#endif // __MQTT_H__
