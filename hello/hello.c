#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#define MPU6050_I2C_ADDRESS 0x68
#define MPU6050_SMPLRT_DIV 0x19
#define MPU6050_CONFIG 0x1A
#define MPU6050_GYRO_CONFIG 0x1B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_ACCEL_XOUT_H 0x3B
#define MPU6050_TEMP_OUT_H 0x41
#define MPU6050_GYRO_XOUT_H 0x43
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_WHO_AM_I 0x75

static void mpu6050_who_am_i() {
    uint8_t val = MPU6050_WHO_AM_I;
    uint8_t buf[1];
    i2c_write_blocking(i2c_default, MPU6050_I2C_ADDRESS, &val, 1, true);
    i2c_write_blocking(i2c_default, MPU6050_I2C_ADDRESS, buf, 1, false);
    printf("WHO_AM_I = %x\n", buf[0]);
}

static void mpu6050_reset() {
    mpu6050_who_am_i();

    uint8_t buf[] = {MPU6050_PWR_MGMT_1, 0x80};
    i2c_write_blocking(i2c_default, MPU6050_I2C_ADDRESS, buf, 2, false);
    sleep_ms(100);

    buf[0] = MPU6050_PWR_MGMT_1;
    buf[1] = 0x00;
    i2c_write_blocking(i2c_default, MPU6050_I2C_ADDRESS, buf, 2, false);

    buf[0] = MPU6050_SMPLRT_DIV;
    buf[1] = 0x07;
    i2c_write_blocking(i2c_default, MPU6050_I2C_ADDRESS, buf, 2, false);

    buf[0] = MPU6050_CONFIG;
    buf[1] = 0x00;
    i2c_write_blocking(i2c_default, MPU6050_I2C_ADDRESS, buf, 2, false);

    buf[0] = MPU6050_GYRO_CONFIG;
    buf[1] = 0x00;
    i2c_write_blocking(i2c_default, MPU6050_I2C_ADDRESS, buf, 2, false);

    buf[0] = MPU6050_ACCEL_CONFIG;
    buf[1] = 0x00;
    i2c_write_blocking(i2c_default, MPU6050_I2C_ADDRESS, buf, 2, false);

    sleep_ms(100);
}

static void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3], int16_t *temp) {
    uint8_t buffer[6];

    uint8_t val = MPU6050_ACCEL_XOUT_H;
    i2c_write_blocking(i2c_default, MPU6050_I2C_ADDRESS, &val, 1, true);
    i2c_read_blocking(i2c_default, MPU6050_I2C_ADDRESS, buffer, 6, false);
    for (int i = 0; i < 3; i++) {
        accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]) / 16384.0;
    }

    val = MPU6050_GYRO_XOUT_H;
    i2c_write_blocking(i2c_default, MPU6050_I2C_ADDRESS, &val, 1, true);
    i2c_read_blocking(i2c_default, MPU6050_I2C_ADDRESS, buffer, 6, false);
    for (int i = 0; i < 3; i++) {
        gyro[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]) / 131.0;
    }

    val = MPU6050_TEMP_OUT_H;
    i2c_write_blocking(i2c_default, MPU6050_I2C_ADDRESS, &val, 1, true);
    i2c_read_blocking(i2c_default, MPU6050_I2C_ADDRESS, buffer, 2, false);

    *temp = buffer[0] << 8 | buffer[1];
}

int main() {
    stdio_init_all();

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    i2c_init(i2c_default, 400 * 1000);

    // GP4 (6 pin)
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    // GP5 (7 pin)
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    // elf ファイルにバイナリ情報を書き込む
    // 2 pin を指定してその機能をバイナリ情報に書き込む
    // 以下のコマンドで確認可能
    // picotools info --pins cmake-build-debug\hello\hello.elf
    // Fixed Pin Information
    //  0:   UART0 TX, UART0 TX
    //  1:   UART0 RX, UART0 RX
    //  4:   I2C0 SDA
    //  5:   I2C0 SCL
    //  25:  On-board LED
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C))
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_UART_RX_PIN, PICO_DEFAULT_UART_TX_PIN, GPIO_FUNC_UART))
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"))

    mpu6050_reset();

    int16_t acceleration[3], gyro[3], temp;
    bool led = 0;

    while (1) {
        mpu6050_read_raw(acceleration, gyro, &temp);
        printf("Acc. X = %2d, Y = %2d, Z = %2d  Gyro. X = %4d, Y = %4d, Z = %4d  Temp. = %f\n", acceleration[0],
               acceleration[1], acceleration[2], gyro[0], gyro[1], gyro[2], (temp / 340.0) + 36.53);
        gpio_put(LED_PIN, led);
        led = !led;
        sleep_ms(100);
    }
}
