/**
 * @file PCF85063A.c
 * @author Victor Alberti Costa (https://github.com/costa-victor)
 * @brief Driver for the RTC PCF85063A:
 * DATASHEET: https://pdf1.alldatasheet.com/datasheet-pdf/view/1255455/NXP/PCF85063A.html
 * @version 0.1
 * @date 2022-03-04
 *
 */

#include "esp_system.h"
#include "PCF85063A.h"
#include "driver/i2c.h"
#include "timegm.h"
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#define BinToBCD(bin) ((((bin) / 10) << 4) + ((bin) % 10))

static esp_err_t last_i2c_err = ESP_OK; 

/**
 * @brief Config and initialize the I2C
 *
 * @return esp_err_t
 */
static esp_err_t i2c_master_driver_initialize(void)
{
	// Check your used i2c port, it may be different according to your project
	i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = 21,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_io_num = 22,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = 100000};
	return i2c_param_config(I2C_NUM_0, &conf);
}


/**
 * @brief Writes an amount of data to the specified register using I2C
 *
 * @param addr Register address
 * @param data Data
 * @param length Data length in bytes
 * @return esp_err_t Error type
 */
esp_err_t PCF_Write(uint8_t addr, uint8_t *data, size_t length)
{

	last_i2c_err = ESP_OK;
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);	// Install and uninstall I2C driver every time
	i2c_master_driver_initialize();
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, PCF8563_WRITE_ADDR, true);
	i2c_master_write_byte(cmd, addr, true);
	i2c_master_write(cmd, data, length, true);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	i2c_driver_delete(I2C_NUM_0);
	last_i2c_err = ret;
	return ret;
}


/**
 * @brief Reads an amount of data to the specified register using I2C
 *
 * @param addr Register address
 * @param data Data
 * @param length Data length in bytes
 * @return esp_err_t Error type
 */
esp_err_t PCF_Read(uint8_t addr, uint8_t *data, size_t length)
{
	last_i2c_err = ESP_OK;
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
	i2c_master_driver_initialize();
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, PCF8563_WRITE_ADDR, true);
	i2c_master_write_byte(cmd, addr, true);
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, PCF8563_READ_ADDR, true);
	i2c_master_read(cmd, data, length, I2C_MASTER_LAST_NACK);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	i2c_driver_delete(I2C_NUM_0);
	last_i2c_err = ret;
	return ret;
}


int PCF_Init(uint8_t mode){
	static bool init = false;
	if(!init){
		uint8_t tmp = 0b00000000;
		esp_err_t ret = PCF_Write(0x00, &tmp, 1);
		if (ret != ESP_OK){
			return -1;
		}
		mode &= 0b00010011;
		ret = PCF_Write(0x01, &mode, 1);
		if (ret != ESP_OK){
			return -2;
		}
		init = true;
	}
	return 0;
}


int PCF_SetDateTime(PCF_DateTime *dateTime) {
	if (dateTime->second >= 60 || dateTime->minute >= 60 || dateTime->hour >= 24 || dateTime->day > 32 || dateTime->weekday > 6 || dateTime->month > 12 || dateTime->year < 1900 || dateTime->year >= 2100)
	{
		return -2;
	}

	uint8_t buffer[7];

	buffer[0] = BinToBCD(dateTime->second) & 0x7F;
	buffer[1] = BinToBCD(dateTime->minute) & 0x7F;
	buffer[2] = BinToBCD(dateTime->hour) & 0x3F;
	buffer[3] = BinToBCD(dateTime->day) & 0x3F;
	buffer[4] = BinToBCD(dateTime->weekday) & 0x07;
	buffer[5] = BinToBCD(dateTime->month) & 0x1F;

	if (dateTime->year >= 2000)
	{
		buffer[5] |= 0x80;
		buffer[6] = BinToBCD(dateTime->year - 2000);
	}
	else
	{
		buffer[6] = BinToBCD(dateTime->year - 1900);
	}

	esp_err_t ret = PCF_Write(0x02, buffer, sizeof(buffer));
	if (ret != ESP_OK) {
		return -1;
	}

	return 0;
}

int PCF_GetDateTime(PCF_DateTime *dateTime) {
	uint8_t buffer[7];
	esp_err_t ret;

	ret = PCF_Read(0x02, buffer, sizeof(buffer));
	if (ret != ESP_OK) {
		return -1;
	}

	dateTime->second = (((buffer[0] >> 4) & 0x07) * 10) + (buffer[0] & 0x0F);
	dateTime->minute = (((buffer[1] >> 4) & 0x07) * 10) + (buffer[1] & 0x0F);
	dateTime->hour = (((buffer[2] >> 4) & 0x03) * 10) + (buffer[2] & 0x0F);
	dateTime->day = (((buffer[3] >> 4) & 0x03) * 10) + (buffer[3] & 0x0F);
	dateTime->weekday = (buffer[4] & 0x07);
	dateTime->month = ((buffer[5] >> 4) & 0x01) * 10 + (buffer[5] & 0x0F);
	dateTime->year = 1900 + ((buffer[6] >> 4) & 0x0F) * 10 + (buffer[6] & 0x0F);

	if (buffer[5] &  0x80)
	{
		dateTime->year += 100;
	}

	if (buffer[0] & 0x80) //Clock integrity not guaranted
	{
		return 1;
	}

	return 0;
}

int PCF_hctosys(){
	int ret;
	PCF_DateTime date = {0};
	struct tm tm = {0};
	struct timeval tv = {0};
	
	ret = PCF_Init(0);
	printf("PCF_Init %d\n", ret);
	if (ret != 0) {
		goto fail;
	}
    ret = PCF_GetDateTime(&date);
	printf("PCF_GetDateTime %d\n", ret);
    if (ret != 0) {
		goto fail;
    }
	tm.tm_sec = date.second;
	tm.tm_min = date.minute;
	tm.tm_hour = date.hour;
	tm.tm_mday = date.day;
	tm.tm_mon = date.month - 1;
	tm.tm_year = date.year - 1900;

	tv.tv_sec = timegm(&tm);
	tv.tv_usec = 0;
	ret = settimeofday(&tv, NULL);
fail:
	return ret;
}

int PCF_systohc(){
	int ret;
	PCF_DateTime date = {0};
	struct tm tm = {0};

	ret = PCF_Init(0);
	if (ret != 0) {
		goto fail;
	}

	time_t now = time(NULL);
	gmtime_r(&now, &tm);
	date.second = tm.tm_sec;
	date.minute = tm.tm_min;
	date.hour = tm.tm_hour;
	date.day = tm.tm_mday;
	date.month = tm.tm_mon + 1;
	date.year = tm.tm_year + 1900;
	date.weekday = tm.tm_wday;

	ret = PCF_SetDateTime(&date);

fail:
	return ret;
}