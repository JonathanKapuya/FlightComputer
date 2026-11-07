#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#define BMP_ADDR 0x77
#define REG_CHIP_ID 0x00
#define REG_PWR_CTRL 0x1B
#define REG_PRESS_DATA 0x04
#define REG_OSR 0x1C
#define REG_ODR 0x1D 



uint8_t read_reg(uint8_t reg) {

    /*true and false decide whether the conversation ends. Since you're using one wire to comm yo
    u don't want it to be taken so you have true in write because you 
    need to recieve the answer in the same convo. when you have read the convo can end with false. write is to write a command
    so that the pico sends the register address to the sensor. Read is when the sensor returns whats on that address. The sensor's chip id is stored at &reg so that's what it
    needs to return us.*/
    i2c_write_blocking(i2c0, BMP_ADDR, &reg, 1, true);
    uint8_t chip_id;
    i2c_read_blocking(i2c0, BMP_ADDR, &chip_id, 1, false);
    return chip_id;


}

void write_reg(uint8_t reg, uint8_t value) {

    uint8_t buf[2] = {reg, value};

    i2c_write_blocking(i2c0, BMP_ADDR, buf, 2, false);


}

uint32_t read_raw_pressure(void) {

    //#define does not write into memory so you need to make a var that holds address of it
    uint8_t reg = REG_PRESS_DATA;

    i2c_write_blocking(i2c0, BMP_ADDR, &reg, 1, true);
    uint8_t data[3];
    i2c_read_blocking(i2c0, BMP_ADDR, data, 3, false);
    uint32_t bytes = (uint32_t) data[2] << 8*2 | data[1] << 8 | data[0];
    return bytes;
}


int main()
{
    stdio_init_all();
    sleep_ms(2000);
    i2c_init(i2c0, 100000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    write_reg(REG_OSR, 0x03);
    write_reg(REG_ODR, 0x03);
    write_reg(REG_PWR_CTRL, 0x33); //0x33 is 0x for hex then each 3 is 0011 which sends command to power everything on and measure cont.

    while (true) {
        printf("%lu\n", read_raw_pressure());
        sleep_ms(1000);
    }

}