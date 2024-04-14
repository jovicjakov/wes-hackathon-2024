#include "my_sntp.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "nvs_flash.h"
#include "esp_netif.h"

static const char *TAG = "sntp";

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 48
#endif

RTC_DATA_ATTR static int boot_count = 0;

static void initialize_time_sync(void);
static void update_time_and_timezone(void);
static void log_current_time(void);
static void setup_sntp(void);

static time_t now;
static struct tm timeinfo;
static char strftime_buf[64];

QueueHandle_t timeQueue;

void updateTimeTask(void *params)
{
    while (1)
    {
        // Update the time
        time(&now);
        localtime_r(&now, &timeinfo);

        // Adjust for timezone if necessary
        setenv("TZ", "CEST", 1);
        tzset();
        localtime_r(&now, &timeinfo);

        timeinfo.tm_hour = timeinfo.tm_hour + 2;
        // Prepare time string
        // strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

        // Send updated time to the queue
        if (xQueueSend(timeQueue, &timeinfo, portMAX_DELAY) != pdPASS)
        {
            ESP_LOGE(TAG, "Failed to send time to the queue");
        }

        // Wait for one minute before updating again
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

void sntp_app_main(void)
{
    ESP_LOGI(TAG, "Boot count: %d", ++boot_count);
    initialize_time_sync();
    update_time_and_timezone();
    log_current_time();
    timeQueue = xQueueCreate(10, sizeof(struct tm));
    xTaskCreate(updateTimeTask, "Update Time Task", 2048, NULL, 5, NULL);
}

static void initialize_time_sync(void)
{
    ESP_LOGI(TAG, "Starting time synchronization");
    nvs_flash_init();
    esp_netif_init();
    setup_sntp();

    // Wait for time to be set
    const int retry_count = 15;
    for (int retry = 0; retry < retry_count && sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET; ++retry)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry + 1, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}

static void update_time_and_timezone(void)
{
    ESP_LOGI(TAG, "Updating time and timezone settings");
    if (timeinfo.tm_year < (2016 - 1900))
    { // Check if time was never set
        time(&now);
        localtime_r(&now, &timeinfo);
    }
    setenv("TZ", "CEST", 1);
    tzset();
}

static void log_current_time(void)
{
    timeinfo.tm_hour = timeinfo.tm_hour + 2;
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "Current date/time: %s", strftime_buf);
}

static void setup_sntp(void)
{
    ESP_LOGI(TAG, "Setting up SNTP");
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "time.windows.com");
    esp_sntp_setservername(1, "pool.ntp.org");

#if LWIP_DHCP_GET_NTP_SRV && SNTP_MAX_SERVERS > 1
    esp_sntp_servermode_dhcp(1); // Accept NTP offers from DHCP server
#endif

    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    esp_sntp_init();

    // Log configured NTP servers
    for (uint8_t i = 0; i < SNTP_MAX_SERVERS; ++i)
    {
        const char *server_name = esp_sntp_getservername(i);
        if (server_name)
        {
            ESP_LOGI(TAG, "NTP Server %d: %s", i, server_name);
        }
        else
        {
            ip_addr_t *ip = (ip_addr_t *)esp_sntp_getserver(i);
            char address_str[INET6_ADDRSTRLEN];
            ipaddr_ntoa_r(ip, address_str, INET6_ADDRSTRLEN);
            ESP_LOGI(TAG, "NTP Server %d: %s", i, address_str);
        }
    }
}
