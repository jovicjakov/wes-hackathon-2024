/**
* @file gui.c

* @brief This file is an example for how to use the LVGL library.
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

//--------------------------------- INCLUDES ----------------------------------
#include "my_mqtt.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include <cJSON.h>
#include "temp_hum_sensor.h"
#include <math.h>

//---------------------------------- MACROS -----------------------------------
#define DELAY_TIME_MS   (1000U)
#define USE_PROPERTY_ARR_SIZE   sizeof(user_property_arr)/sizeof(esp_mqtt5_user_property_item_t)

//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------
/**
 * @brief It creates a new demo application.
 */
static void _create_demo_application(void);

static const char *TAG = "MY_MQTT";

static void mqtt5_app_start(void);
//------------------------- STATIC DATA & CONSTANTS ---------------------------
typedef enum
{
   DEVICE,
   SERVER
} tictactoe_turn_t;

typedef struct {
    int indexX[9];  // Places where 'X' is marked
    int indexO[9];  // Places where 'O' is marked
    tictactoe_turn_t turn;  // Whose turn is it
} tictactoe_handler_t;

//------------------------------- GLOBAL DATA ---------------------------------
esp_mqtt_client_handle_t client;
extern QueueHandle_t p_tictactoe_queue;

//------------------------------ PUBLIC FUNCTIONS -----------------------------
void my_mqtt_init()
{  
    //xTaskCreate(mqtt_publish_task, "MQTT Publish Task", 2048, NULL, 10, NULL);

    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("mqtt_example", ESP_LOG_VERBOSE);
    esp_log_level_set("transport_base", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("transport", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    mqtt5_app_start();

}

//---------------------------- PRIVATE FUNCTIONS ------------------------------
static char* create_json_payload(void) {
    // Create the root object to hold the data
    cJSON *root = cJSON_CreateObject();

    float temp = floorf(floorf(getTemperature() * 100) / 100);
    float hum = floorf(floorf(getHumidity() * 100) / 100);

    // Add temperature and humidity directly to the root object
    cJSON_AddNumberToObject(root, "temp", temp);
    cJSON_AddNumberToObject(root, "hum", hum);

    // Create a nested object for accelerometer data
    cJSON *acc = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "acc", acc);
    cJSON_AddNumberToObject(acc, "x", 0.5);
    cJSON_AddNumberToObject(acc, "y", -0.3);
    cJSON_AddNumberToObject(acc, "z", 0.1);

    // Serialize the JSON object to a string
    char *formatted_json = cJSON_Print(root);

    //ESP_LOGI(TAG, "%s", formatted_json);

    // Clean up and free memory
    cJSON_Delete(root);

    return formatted_json;
}

static char* create_json_payload_game(tictactoe_handler_t *game_data) {
    cJSON *root = cJSON_CreateObject();

    cJSON *jsonIndexX = cJSON_CreateIntArray(game_data->indexX, 9);
    cJSON *jsonIndexO = cJSON_CreateIntArray(game_data->indexO, 9);
    const char *turnString = game_data->turn == 0 ? "device" : "server";

    cJSON_AddItemToObject(root, "indexX", jsonIndexX);
    cJSON_AddItemToObject(root, "indexO", jsonIndexO);
    cJSON_AddStringToObject(root, "turn", turnString);

    char *formatted_json = cJSON_Print(root);
    printf("JSON Output: %s\n", formatted_json);

    cJSON_Delete(root);
    return formatted_json;
}

//---------------------------- EVENT HANDLERS -----------------------------

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt5_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;
    char topic[20];
    char data[2500];

    switch ((esp_mqtt_event_id_t)event_id) {

        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            char *json_payload = create_json_payload();
    
            ESP_LOGI(TAG, "Publishing to topic WES/Uranus/sensors on INIT");
            esp_mqtt_client_publish(client, "WES/Uranus/sensors", json_payload, 0, 1, 0); 
            
            free(json_payload);
            esp_mqtt_client_subscribe(client, "WES/Uranus/game", 0);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA Received");
            printf("Received: Topic=%.*s, Data=%.*s\n", event->topic_len, event->topic, event->data_len, event->data);

            // Parse the JSON payload from the received data
            cJSON *root = cJSON_ParseWithLength(event->data, event->data_len);
            if (root == NULL) {
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL) {
                    fprintf(stderr, "Error before: %s\n", error_ptr);
                }
                ESP_LOGE(TAG, "Failed to parse JSON");
            } else {
                // Extract the game state from the JSON
                cJSON *indexX = cJSON_GetObjectItemCaseSensitive(root, "indexX");
                cJSON *indexO = cJSON_GetObjectItemCaseSensitive(root, "indexO");
                cJSON *turn = cJSON_GetObjectItemCaseSensitive(root, "turn");

                if (cJSON_IsArray(indexX) && cJSON_IsArray(indexO) && cJSON_IsString(turn)) {
                    printf("Turn: %s\n", turn->valuestring);
                    printf("IndexX: ");
                    for (int i = 0; i < cJSON_GetArraySize(indexX); i++) {
                        cJSON *ix = cJSON_GetArrayItem(indexX, i);
                        if (cJSON_IsNumber(ix)) {
                            printf("%d ", ix->valueint);
                        }
                    }
                    printf("\nIndexO: ");
                    for (int i = 0; i < cJSON_GetArraySize(indexO); i++) {
                        cJSON *io = cJSON_GetArrayItem(indexO, i);
                        if (cJSON_IsNumber(io)) {
                            printf("%d ", io->valueint);
                        }
                    }
                    printf("\n");
                } else {
                    ESP_LOGE(TAG, "JSON does not contain the expected structure or types");
                }

                // Free the JSON object
                cJSON_Delete(root);
            }
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;

        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
            
}

static void mqtt5_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://192.168.254.6:1883",
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt5_event_handler, NULL);
    esp_mqtt_client_start(client);
    
}

static void my_mqtt_task(void *pvParameters) {
    char* msg;
    tictactoe_handler_t tictactoe_msg;

    for(;;) {
        //if(xQueueReceive(temp_queue, &msg, portMAX_DELAY) == pdPASS) {
            // Temp threshold reached, publish it
            //ESP_LOGI(TAG, "Motion detected, publishing to MQTT");
            //esp_mqtt_client_publish("your/mqtt/topic", msg, 0, 0); // Adjust topic, QoS, retain as needed
        //}

        // if(xQueueRecieve(p_tictactoe_queue, &tictactoe_msg, portMAX_DELAY) == pdPASS) {
        //     ESP_LOGI(TAG, "Publishing move!");
        //     char *json_payload = create_json_payload_game(&tictactoe_msg);
        //     esp_mqtt_client_publish(client, "WES/Uranus/game", json_payload, 0, 1, 0);
        // }
    }
}

//---------------------------- INTERRUPT HANDLERS -----------------------------
