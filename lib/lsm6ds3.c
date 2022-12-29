#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/pio.h"
#include "lsm6ds3.h"
#include "pio_i2c.h"


void LSM6DS3_init(PIO pio, uint sm) {
    uint8_t command[2];

    // set acceleration sensor
    command[0] = LSM6DS3_CTRL1_XL;
    command[1] = 0x4A;  //set 104Hz, 0100 1010
    pio_i2c_write_blocking(pio, sm, LSM6DS3_ADDRESS, command, 2, false);

    command[0] = LSM6DS3_CTRL8_XL;
    command[1] = 0x09;
    pio_i2c_write_blocking(pio, sm, LSM6DS3_ADDRESS, command, 2, false);

    // set gyro sensor
    command[0] = LSM6DS3_CTRL2_G;  //shutdown
    command[1] = 0x4C;
    pio_i2c_write_blocking(pio, sm, LSM6DS3_ADDRESS, command, 2, false);

    command[0] = LSM6DS3_CTRL7_G;
    command[1] = 0x00;
    pio_i2c_write_blocking(pio,sm, LSM6DS3_ADDRESS, command, 2, false);
}

void readAcceleration(PIO pio, uint sm, int16_t* data) {

    uint8_t buf[6];
    uint8_t reg = LSM6DS3_OUTX_L_XL;
    pio_i2c_write_blocking(pio, sm, LSM6DS3_ADDRESS, &reg, 1, true);
    pio_i2c_read_blocking(pio, sm, LSM6DS3_ADDRESS, buf, 6, false);
    
    for (int i = 0; i < 3; i++){
        data[i] = (buf[2*i+1] << 8) | buf[2*i];
    }
    
    /*
    data[0] = (buf[1] << 8) | buf[0];
    data[1] = (buf[3] << 8) | buf[2];
    data[2] = (buf[5] << 8) | buf[4];
    */
}

void readGyro(PIO pio, uint sm, int16_t* data) {

    uint8_t buf[6];
    uint8_t reg = LSM6DS3_OUTX_L_G;
    pio_i2c_write_blocking(pio, sm, LSM6DS3_ADDRESS, &reg, 1, true);
    pio_i2c_read_blocking(pio, sm, LSM6DS3_ADDRESS, buf, 6, false);
    
    for (int i = 0; i < 3; i++){
        data[i] = (buf[2*i+1] << 8) | buf[2*i];
    }
    
    /*
    data[0] = (buf[1] << 8) | buf[0];
    data[1] = (buf[3] << 8) | buf[2];
    data[2] = (buf[5] << 8) | buf[4];
    */
}