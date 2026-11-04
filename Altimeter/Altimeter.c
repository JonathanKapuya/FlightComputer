#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#define BMP_ADDR 0x77
#define REG_CHIP_ID 0x00

uint8_t read_chip_id(void) {

    /*true and false decide whether the conversation ends. Since you're using one wire to comm yo
    u don't want it to be taken so you have true in write because you 
    need to recieve the answer in the same convo. when you have read the convo can end with false. write is to write a command
    so that the sensor sends a byte to reg. Read is when the sensor returns that. The sensor's chip id is stored at &reg so that's what it
    needs to return us.*/
    uint8_t reg = REG_CHIP_ID;
    i2c_write_blocking(i2c0, BMP_ADDR, &reg, 1, true);
    uint8_t chip_id;
    i2c_read_blocking(i2c0, BMP_ADDR, &chip_id, 1, false);
    return chip_id;


}


int main()
{
    stdio_init_all();
    sleep_ms(2000);
    i2c_init(i2c0, 100000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);


    while (true) {
        printf("%02X\n", read_chip_id());
        sleep_ms(1000);
    }

}