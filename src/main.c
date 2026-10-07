

// IWYU pragma: keep
//
/*
#include "freertos/task.h"
#include <stdio.h>
#include <driver/i2c.h>
#include "esp_pm.h"


Example program that reads data from MPU6050
Connections:
Vcc -> 3.3V
GND -> GND
SCL -> 32
SDA -> 33
AD0 -> GND
From the Datasheet, page 33:
The slave address of the MPU-60X0 is b110100X which is 7 bits long.
The LSB bit of the 7 bit address is determined by the logic level on pin AD0.
This allows two MPU-60X0s to be connected to the same I2C bus.
// If AD0 is LOW, the I2C address of the board will be 0x68. Otherwise, the address will be 0x69.
// Bit1 is reset accelerometer
#define MPU6050_ADDR 0x68

From Datasheet, page 40:
Register (Hex): 6B
Bit7 DEVICE_RESET
Bit6 SLEEP
Bit5 CYCLE
Bit4 -
Bit3 TEMP_DIS
Bit2, Bit1, Bit0 CLKSEL[2:0]
#define MPU6050_PWR_MGMT_1 0x6B

Register (Hex) 19
Bit7, Bit6, Bit5, Bit4, Bit3, Bit2, Bit1, Bit0: SMPLRT_DIV[7:0]
#define MPU6050_SMPLRT_DIV 0x19

From Datasheet, page 45:
Register (Hex): 75
Bit7 -
Bit6, Bit5, Bit4, Bit3, Bit2, Bit1: WHO_AM_I[6:1]
Bit0: -
#define MPU6050_WHO_AM_I 0x75

From Datasheet, page 30:
Register (Hex) 41
Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0: TEMP_OUT[15:8]
Register (Hex) 42
Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0: TEMP_OUT[7:0]
#define MPU6050_TEMP_OUT_H 0x41
#define MPU6050_TEMP_OUT_L 0x42


#define MPU6050_ACCEL_XOUT_H 0x3B
#define MPU6050_ACCEL_XOUT_L 0x3C
#define MPU6050_ACCEL_YOUT_H 0x3D
#define MPU6050_ACCEL_YOUT_L 0x3E
#define MPU6050_ACCEL_ZOUT_H 0x3F
#define MPU6050_ACCEL_ZOUT_L 0x40

void app_main(void) {



    esp_pm_config_t pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 80,
        .light_sleep_enable = true,
    };

    esp_err_t err = esp_pm_configure(&pm_config);
    ESP_ERROR_CHECK(err);


  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;             // ESP32 acts as master
  conf.sda_io_num = 33;                    // pin used for SDA
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE; // The SDA and SCL lines are active low, so they should be pulled up with resistors
  conf.scl_io_num = 32;                    // SCL pin number
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE; // enables pullup on SDA
  conf.master.clk_speed = 100000;          // Standard mode (100 Kbit/s)
  // configure I2C controller 0
  esp_err_t res = i2c_param_config(I2C_NUM_0, &conf);
  ESP_ERROR_CHECK(res);
  // install driver, no buffers needed in master mode nor special interrupts config
  res = i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
  ESP_ERROR_CHECK(res);
  // configure power mode
  // here we set all bits of the PWR_MGMT_1 register to 0

  // create command
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  // start command
  res = i2c_master_start(cmd);
  ESP_ERROR_CHECK(res);
  // set address + write and check for ack
  res = i2c_master_write_byte(cmd, MPU6050_ADDR << 1 | I2C_MASTER_WRITE, 1);
  ESP_ERROR_CHECK(res);
  // write the register address and check for ack
  res = i2c_master_write_byte(cmd, MPU6050_PWR_MGMT_1, 1);
  ESP_ERROR_CHECK(res);
  // write value of the regiter: 0, and check for ack
  res = i2c_master_write_byte(cmd, 0x00, 1);
  ESP_ERROR_CHECK(res);
  // end of command
  res = i2c_master_stop(cmd);
  ESP_ERROR_CHECK(res);
  // send the command, 1 second timeout
  res = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
  ESP_ERROR_CHECK(res);
  // delete command now that it's not needed
  i2c_cmd_link_delete(cmd);

  // set the sampling frequency
  // the sampling freq is gyro sampling freq / (1 + divider)
  // setting divider to 250 leads to sampling freq. of 32 Hz
  cmd = i2c_cmd_link_create();
  res = i2c_master_start(cmd);
  ESP_ERROR_CHECK(res);
  res = i2c_master_write_byte(cmd, MPU6050_ADDR << 1 | I2C_MASTER_WRITE, 1); // WRITE bit set!
  ESP_ERROR_CHECK(res);
  res = i2c_master_write_byte(cmd, MPU6050_SMPLRT_DIV, 1); // write to SMPLRT_DIV
  ESP_ERROR_CHECK(res);
  res = i2c_master_write_byte(cmd, 250, 1); // set SMPLRT_DIV to 250
  ESP_ERROR_CHECK(res);
  res = i2c_master_stop(cmd);
  ESP_ERROR_CHECK(res);
  res = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
  // ESP_ERROR_CHECK(res);
  i2c_cmd_link_delete(cmd);

  while (1)
  {
      // read the latest sampled temperature
      // we need to combine TEMP_OUT_L and TEMP_OUT_H into one 16-bit signed integer
      // and then coonvert that into C using the formula: t = temp_out /340 + 36.53

      // create a little buffer where to store the answer
      uint8_t bufferX;
      uint8_t bufferY;
      uint8_t bufferZ;
      // holder of the temperature
      int16_t accelX = 0;
      int16_t accelY = 0;
      int16_t accelZ = 0;



      // read low register
      // send just the register number with no other data
      // Z axis
      cmd = i2c_cmd_link_create();
      res = i2c_master_start(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_write_byte(cmd, MPU6050_ADDR << 1 | I2C_MASTER_WRITE, 1); // WRITE bit set!
      ESP_ERROR_CHECK(res);
      res = i2c_master_write_byte(cmd, MPU6050_ACCEL_ZOUT_L, 1); // read low first
      ESP_ERROR_CHECK(res);
      res = i2c_master_stop(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
      ESP_ERROR_CHECK(res);
      i2c_cmd_link_delete(cmd);

      // wait a little
      vTaskDelay(pdMS_TO_TICKS(10));

      // now read the answer
      cmd = i2c_cmd_link_create();
      res = i2c_master_start(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_write_byte(cmd, MPU6050_ADDR << 1 | I2C_MASTER_READ, 1); // READ bit set!
      ESP_ERROR_CHECK(res);
      res = i2c_master_read(cmd, &bufferZ, 1, I2C_MASTER_NACK);
      ESP_ERROR_CHECK(res);
      res = i2c_master_stop(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
      ESP_ERROR_CHECK(res);
      i2c_cmd_link_delete(cmd);

      accelZ = bufferZ;

      // read high register
      cmd = i2c_cmd_link_create();
      res = i2c_master_start(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_write_byte(cmd, MPU6050_ADDR << 1 | I2C_MASTER_WRITE, 1); // WRITE bit set!
      ESP_ERROR_CHECK(res);
      res = i2c_master_write_byte(cmd, MPU6050_ACCEL_ZOUT_H, 1); // read high
      ESP_ERROR_CHECK(res);
      res = i2c_master_stop(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
      ESP_ERROR_CHECK(res);
      i2c_cmd_link_delete(cmd);

      vTaskDelay(pdMS_TO_TICKS(10));

      cmd = i2c_cmd_link_create();
      res = i2c_master_start(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_write_byte(cmd, MPU6050_ADDR << 1 | I2C_MASTER_READ, 1); // READ bit set!
      ESP_ERROR_CHECK(res);
      res = i2c_master_read(cmd, &bufferZ, 1, I2C_MASTER_NACK);
      ESP_ERROR_CHECK(res);
      res = i2c_master_stop(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
      ESP_ERROR_CHECK(res);
      i2c_cmd_link_delete(cmd);

      // read low register
      // send just the register number with no other data
      cmd = i2c_cmd_link_create();
      res = i2c_master_start(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_write_byte(cmd, MPU6050_ADDR << 1 | I2C_MASTER_WRITE, 1); // WRITE bit set!
      ESP_ERROR_CHECK(res);
      res = i2c_master_write_byte(cmd, MPU6050_ACCEL_XOUT_L, 1); // read low first
      ESP_ERROR_CHECK(res);
      res = i2c_master_stop(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
      ESP_ERROR_CHECK(res);
      i2c_cmd_link_delete(cmd);

      // wait a little
      vTaskDelay(pdMS_TO_TICKS(10));

      // now read the answer
      cmd = i2c_cmd_link_create();
      res = i2c_master_start(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_write_byte(cmd, MPU6050_ADDR << 1 | I2C_MASTER_READ, 1); // READ bit set!
      ESP_ERROR_CHECK(res);
      res = i2c_master_read(cmd, &bufferX, 1, I2C_MASTER_NACK);
      ESP_ERROR_CHECK(res);
      res = i2c_master_stop(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
      ESP_ERROR_CHECK(res);
      i2c_cmd_link_delete(cmd);

      accelX = bufferX;

      // read high register
      cmd = i2c_cmd_link_create();
      res = i2c_master_start(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_write_byte(cmd, MPU6050_ADDR << 1 | I2C_MASTER_WRITE, 1); // WRITE bit set!
      ESP_ERROR_CHECK(res);
      res = i2c_master_write_byte(cmd, MPU6050_ACCEL_XOUT_H, 1); // read high
      ESP_ERROR_CHECK(res);
      res = i2c_master_stop(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
      ESP_ERROR_CHECK(res);
      i2c_cmd_link_delete(cmd);

      vTaskDelay(pdMS_TO_TICKS(10));

      cmd = i2c_cmd_link_create();
      res = i2c_master_start(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_write_byte(cmd, MPU6050_ADDR << 1 | I2C_MASTER_READ, 1); // READ bit set!
      ESP_ERROR_CHECK(res);
      res = i2c_master_read(cmd, &bufferX, 1, I2C_MASTER_NACK);
      ESP_ERROR_CHECK(res);
      res = i2c_master_stop(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
      ESP_ERROR_CHECK(res);
      i2c_cmd_link_delete(cmd);


      // read low register
      // send just the register number with no other data
      cmd = i2c_cmd_link_create();
      res = i2c_master_start(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_write_byte(cmd, MPU6050_ADDR << 1 | I2C_MASTER_WRITE, 1); // WRITE bit set!
      ESP_ERROR_CHECK(res);
      res = i2c_master_write_byte(cmd, MPU6050_ACCEL_YOUT_L, 1); // read low first
      ESP_ERROR_CHECK(res);
      res = i2c_master_stop(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
      ESP_ERROR_CHECK(res);
      i2c_cmd_link_delete(cmd);

      // wait a little
      vTaskDelay(pdMS_TO_TICKS(10));

      // now read the answer
      cmd = i2c_cmd_link_create();
      res = i2c_master_start(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_write_byte(cmd, MPU6050_ADDR << 1 | I2C_MASTER_READ, 1); // READ bit set!
      ESP_ERROR_CHECK(res);
      res = i2c_master_read(cmd, &bufferY, 1, I2C_MASTER_NACK);
      ESP_ERROR_CHECK(res);
      res = i2c_master_stop(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
      ESP_ERROR_CHECK(res);
      i2c_cmd_link_delete(cmd);

      accelY = bufferY;

      // read high register
      cmd = i2c_cmd_link_create();
      res = i2c_master_start(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_write_byte(cmd, MPU6050_ADDR << 1 | I2C_MASTER_WRITE, 1); // WRITE bit set!
      ESP_ERROR_CHECK(res);
      res = i2c_master_write_byte(cmd, MPU6050_ACCEL_YOUT_H, 1); // read high
      ESP_ERROR_CHECK(res);
      res = i2c_master_stop(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
      ESP_ERROR_CHECK(res);
      i2c_cmd_link_delete(cmd);

      vTaskDelay(pdMS_TO_TICKS(10));

      cmd = i2c_cmd_link_create();
      res = i2c_master_start(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_write_byte(cmd, MPU6050_ADDR << 1 | I2C_MASTER_READ, 1); // READ bit set!
      ESP_ERROR_CHECK(res);
      res = i2c_master_read(cmd, &bufferY, 1, I2C_MASTER_NACK);
      ESP_ERROR_CHECK(res);
      res = i2c_master_stop(cmd);
      ESP_ERROR_CHECK(res);
      res = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
      ESP_ERROR_CHECK(res);
      i2c_cmd_link_delete(cmd);
      // combine high and low registers into a signed integer
      accelZ |= ((int16_t)bufferZ) << 8;
      accelY |= ((int16_t)bufferY) << 8;
      accelX |= ((int16_t)bufferX) << 8;

      printf("z-accel raw is: %d\n", accelZ);
      printf("y-accel raw is: %d\n", accelY);
      printf("x-accel raw is: %d\n", accelX);

      // convert raw value to temperature
      float accZms2 = ((float)accelZ) / 16384.0 * 9.80665;
      float accYms2 = ((float)accelY) / 16384.0 * 9.80665;
      float accXms2 = ((float)accelX) / 16384.0 * 9.80665;
      printf("z-accel is: %.2f m/s^2\n", accZms2);
      printf("y-accel is: %.2f m/s^2\n", accYms2);
      printf("x-accel is: %.2f m/s^2\n", accXms2);

      // wait and repeat
      vTaskDelay(pdMS_TO_TICKS(1000));
  }

}

*/



#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_pm.h"
#include "esp_err.h"
#include <rom/ets_sys.h>
#include <math.h>
#include "driver/gpio.h"
#include <hal/gpio_types.h>
#include <freertos/semphr.h>
#include <esp_timer.h>
#include <esp_log.h>


#include "communication.h"
#include "accelerometer.h"
#include "buffer.h"


/*
 *
 * With MIN_SD=50 and K=2, the threshold is 2 standard deviations above the mean. This means
 * that the algorithm will detect a step when the acceleration exceeds 2 standard deviations
 * above the mean. This is good for a step detection algorithm.
 *
 *
 */

#define TASK_NAME_SAMPLING "sampling_task"
#define TASK_NAME_ALGO "algo_task"
#define TASK_NAME_BUTTON "button_task"
#define TASK_STACK_SIZE 8192
#define TASK_PRIORITY 10
#define SAMPLING_PERIOD 20
#define BUFFER_SIZE 128
#define ALGO_PERIOD 2000
#define ALGO_MAX_SAMPLES 128
#define MIN_SD 50
#define K 2
#define MIN_INTRA_STEP_TIME 10
#define STEPS_GOAL 10
#define LED_PIN 22
#define BUTTON_PIN 21
#define PUSH_TIME_US 250000


static volatile int64_t last_button_press = -PUSH_TIME_US;
static const char *TAG_BTN = "BTN";
static const char *TAG_SAMPLE = "SAMPLE";
static const char *TAG_ALGO   = "ALGO";

buffer_t buffer;
int step_count = 0;


SemaphoreHandle_t step_count_mutex = NULL;
SemaphoreHandle_t buffer_mutex = NULL;


/*
 * Running pace is 160 steps per minute which is 160/60 = 2.67 steps per second.
 * This means the frequency is 2.67 Hz. Nyquist frequency is half this, so 1.33 Hz.
 */

static void sampling_task(void *pvParameters)
{
    TickType_t last_wake_time = xTaskGetTickCount();
    int tick = 0;
    while (1) {
        acceleration_t acc = readAcceleration();

        float mag = sqrtf((float)acc.x * (float)acc.x
                              + (float)acc.y * (float)acc.y
                              + (float)acc.z * (float)acc.z);
        uint32_t magnitude = (uint32_t)(mag * 1000.0f);
        xSemaphoreTake(buffer_mutex, portMAX_DELAY);
        buffer_add(&buffer, magnitude);
        uint32_t n = buffer_number_elements(&buffer);
        xSemaphoreGive(buffer_mutex);
        //buffer_print(&buffer);

        if ((tick++ % 25) == 0) {
                    ESP_LOGI(TAG_SAMPLE, "mag=%lu mg  buf=%lu",
                             (unsigned long)magnitude, (unsigned long)n);
                }
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(SAMPLING_PERIOD));
    }
}

static void algo_task(void *pvParameters)
{
    TickType_t last_wake_time = xTaskGetTickCount();
    uint32_t local[ALGO_MAX_SAMPLES];
    while (1) {
        int size = 0;

        xSemaphoreTake(buffer_mutex, portMAX_DELAY);
        int n = buffer_number_elements(&buffer);
        if (n > ALGO_MAX_SAMPLES) n = ALGO_MAX_SAMPLES;
        for (int i = 0; i < n; i++) {
            local[i] = buffer_pop(&buffer);
        }
        size = n;
        xSemaphoreGive(buffer_mutex);
        ESP_LOGI(TAG_ALGO, "drained %d samples", size);
        if(size > 0) {
            float mean = 0.0f;
            for (int i = 0; i < size; i++) mean += local[i];
            mean /= size;

            float sd = 0.0f;
            for (int i = 0; i < size; i++) {
                float d = local[i] - mean;
                sd += d * d;
            }
            sd = sqrtf(sd / size);
            if (sd < MIN_SD) sd = MIN_SD;
            float threshold = mean + K * sd;

            uint32_t max = local[0];
            uint32_t min = local[0];
            for (int i = 1; i < size; i++) {
                if (local[i] > max) max = local[i];
                if (local[i] < min) min = local[i];
            }

            ESP_LOGI(TAG_ALGO, "min=%lu max=%lu mean=%ld sd=%ld thresh=%ld",
                     (unsigned long)min, (unsigned long)max,
                     (long)mean, (long)sd, (long)threshold);



            int steps_this_window = 0;
            int last_step_index = -MIN_INTRA_STEP_TIME;
            for (int i = 0; i < size; i++) {
                if (local[i] > threshold
                    && (i - last_step_index) > MIN_INTRA_STEP_TIME) {
                    step_count++;
                    steps_this_window++;
                    last_step_index = i;
                }
            }
            ESP_LOGI(TAG_ALGO, "steps this window=%d  total=%d",
                     steps_this_window, step_count);
        }
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(ALGO_PERIOD));
}
}

static void button_isr_handler(void *arg) {

    uint64_t now = esp_timer_get_time();

    if(now - last_button_press > PUSH_TIME_US) {
        last_button_press = now;

        xSemaphoreGiveFromISR(step_count_mutex, NULL);
    }

}


static void button_task(void *arg) {
    while(1) {
        if(xSemaphoreTake(step_count_mutex, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG_BTN, "button pressed, step_count=%d", step_count);
            if (step_count > STEPS_GOAL) {
                ESP_LOGI(TAG_BTN, "pattern start, step_count=%d", step_count);
                for (int i = 0; i < 10; i++) {
                    gpio_set_level(LED_PIN, 1);
                    ESP_LOGI(TAG_BTN, "LED HIGH");
                    vTaskDelay(pdMS_TO_TICKS(300));
                    gpio_set_level(LED_PIN, 0);
                    ESP_LOGI(TAG_BTN, "LED LOW");
                    vTaskDelay(pdMS_TO_TICKS(300));
                }
            } else {
                gpio_set_level(LED_PIN, 0);
            }
        }
    }
}

void app_main(void)
{
    buffer_init(&buffer, BUFFER_SIZE);
    step_count_mutex = xSemaphoreCreateBinary();
    buffer_mutex = xSemaphoreCreateMutex();


    if (step_count_mutex == NULL || buffer_mutex == NULL) {
        printf("failed to create mutexes\n");
        return;
    }


    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);

    gpio_config_t io_config = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    esp_err_t error = gpio_config(&io_config);
    if (error != ESP_OK) {
        printf("gpio_config failed: %s\n", esp_err_to_name(error));
        return;
    }

    esp_pm_config_t pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 80,
        .light_sleep_enable = true,
    };

    esp_err_t err = esp_pm_configure(&pm_config);
    if (err != ESP_OK) {
        printf("esp_pm_configure failed: %s\n", esp_err_to_name(err));
        return;
    }

    // SDA = GPIO 33, SCL = GPIO 32
    initI2C(33, 32);

    initAccelerometer();

    BaseType_t ret = xTaskCreate(sampling_task, TASK_NAME_SAMPLING, TASK_STACK_SIZE, NULL, TASK_PRIORITY, NULL);
    if (ret != pdPASS) {
        printf("xTaskCreate failed");
    }

    BaseType_t ret2 = xTaskCreate(algo_task, TASK_NAME_ALGO, TASK_STACK_SIZE, NULL, TASK_PRIORITY, NULL);
    if (ret2 != pdPASS) {
        printf("xTaskCreate failed");
    }

    BaseType_t ret3 = xTaskCreate(button_task, TASK_NAME_BUTTON, TASK_STACK_SIZE, NULL, TASK_PRIORITY, NULL);
    if (ret3 != pdPASS) {
        printf("xTaskCreate failed");
    }
}
