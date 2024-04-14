#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "esp_sntp.h"

typedef struct currentTimeInfo {
    int hour;
    int min;
    int sec;
    int day;
    int weekDay;
    int month;
    int year;
} currentTimeInfo;

void sntp_app_main(void);
currentTimeInfo *fetchTime();