#include "communication.h"

#include "driver/i2c.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"

#define I2C_MASTER_TIMEOUT_MS 1000

void initI2C(int sdapin, int sclpin)
{
    i2c_config_t conf = {0};

    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sdapin;
    conf.scl_io_num = sclpin;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000; // 100 kHz

    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0));
}

void writeI2C(uint8_t address, uint8_t reg, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, data, true));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));

    ESP_ERROR_CHECK(i2c_master_cmd_begin(
        I2C_NUM_0, cmd, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS)));

    i2c_cmd_link_delete(cmd);
}

void readI2C(uint8_t address, uint8_t reg, uint8_t *buffer, int len)
{
    if (len <= 0) {
        return;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg, true));

    // Repeated start, then read.
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_READ, true));

    if (len > 1) {
        ESP_ERROR_CHECK(i2c_master_read(cmd, buffer, len - 1, I2C_MASTER_ACK));
    }

    ESP_ERROR_CHECK(i2c_master_read(cmd, buffer + len - 1, 1, I2C_MASTER_NACK));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));

    ESP_ERROR_CHECK(i2c_master_cmd_begin(
        I2C_NUM_0, cmd, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS)));

    i2c_cmd_link_delete(cmd);
}
