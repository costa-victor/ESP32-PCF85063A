/**
 * @file example.c
 * @author Victor Alberti Costa (https://github.com/costa-victor)
 * @brief Simple example of using this driver for the RTC PCF85063A
 * DATASHEET: https://pdf1.alldatasheet.com/datasheet-pdf/view/1255455/NXP/PCF85063A.html
 * @version 0.1
 * @date 2022-03-05
 *
 */

#include "PCF85063A.h"
#include "timegm.h"
#include "driver/i2c.h"
#include "esp_log.h"

/**
 *  Global variables
 */
static const char *TAG = "RTC";
struct tm rtcTime = {0};

/**
 * @brief Prints the current RTC and system time
 * 
 */
void print_current_time(void)
{
    PCF_DateTime currentSystem = {0}, currentRTC = {0};
    struct tm tm = {0};
    time_t now = time(NULL);
    gmtime_r(&now, &tm);
    currentSystem.second = tm.tm_sec;
    currentSystem.minute = tm.tm_min;
    currentSystem.hour = tm.tm_hour;
    currentSystem.day = tm.tm_mday;
    currentSystem.month = tm.tm_mon + 1;
    currentSystem.year = tm.tm_year + TM_YEAR_BASE;
    currentSystem.weekday = tm.tm_wday;

    ESP_LOGW(TAG, "...::: Current system time :::...");
    ESP_LOGI(TAG, "second  : %d", currentSystem.second);
    ESP_LOGI(TAG, "minute  : %d", currentSystem.minute);
    ESP_LOGI(TAG, "hour    : %d", currentSystem.hour);
    ESP_LOGI(TAG, "day     : %d", currentSystem.day);
    ESP_LOGI(TAG, "month   : %d", currentSystem.month);
    ESP_LOGI(TAG, "year    : %d", currentSystem.year);
    ESP_LOGI(TAG, "weekday : %d", currentSystem.weekday);

    PCF_GetDateTime(&currentRTC);
    ESP_LOGW(TAG, "...::: Current RTC time :::...");
    ESP_LOGI(TAG, "second  : %d", currentRTC.second);
    ESP_LOGI(TAG, "minute  : %d", currentRTC.minute);
    ESP_LOGI(TAG, "hour    : %d", currentRTC.hour);
    ESP_LOGI(TAG, "day     : %d", currentRTC.day);
    ESP_LOGI(TAG, "weekday : %d", currentRTC.weekday);
    ESP_LOGI(TAG, "month   : %d", currentRTC.month);
    ESP_LOGI(TAG, "year    : %d", currentRTC.year);
}

void app_main(void)
{
    struct tm *newTime = &rtcTime;
    time_t now;
    int err;

    /* ...::: Updates the RTC time - example code :::... */
    time(&now);
    newTime = localtime(&now);
	newTime->tm_sec = 0;
	newTime->tm_min = 25;
	newTime->tm_hour = 16;
	newTime->tm_mday = 6;
	newTime->tm_mon = 3 - 1;                    // Month range: 0 - 11
	newTime->tm_year = 2022 - TM_YEAR_BASE;
    
    err = PCF_updateRTC(newTime);               // Updates RTC with your new timestamp

    if (err == -1)
    {
        ESP_LOGE(TAG, "Failed to update RTC time - Fail during write in registers");
    }
    else if (err == -2)
    {
        ESP_LOGE(TAG, "%s - Failed to update the RTC time - Invalid date parameters", __ASSERT_FUNC);
    }

    /* Check the system and the RTC time after the update */
    print_current_time();
}