#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#define BMP_ADDR 0x77
#define REG_CHIP_ID 0x00
#define REG_PWR_CTRL 0x1B
#define REG_PRESS_DATA 0x04
#define REG_OSR 0x1C
#define REG_ODR 0x1D 
#define REG_TEMP_DATA 0x07
#define REG_CALIB 0x31
#define REG_STATUS 0x03
#include <math.h>
//~/.pico-sdk/picotool/*/picotool/picotool load -x -f build/Altimeter.uf2

typedef struct calib_t {

    uint16_t t1;
    uint16_t t2;
    int8_t t3;
    int16_t p1;
    int16_t p2;
    int8_t p3;
    int8_t p4;
    uint16_t p5;
    uint16_t p6;
    int8_t p7;
    int8_t p8;
    int16_t p9;
    int8_t p10;
    int8_t p11;

} calib_t;

calib_t parse_calib(uint8_t *c) {

    calib_t out;

    out.t1 = (uint16_t)c[1] << 8 | c[0];
    out.t2 = (uint16_t)c[3] << 8 | c[2];
    out.t3 = (int8_t)c[4];
    out.p1 = (int16_t)((uint16_t)c[6] << 8 | c[5]);
    out.p2 = (int16_t)((uint16_t)c[8] << 8 | c[7]);
    out.p3 = (int8_t)c[9];
    out.p4 = (int8_t)c[10];
    out.p5 = (uint16_t)c[12] << 8 | c[11];
    out.p6 = (uint16_t)c[14] << 8 | c[13];
    out.p7 = (int8_t)c[15];
    out.p8 = (int8_t)c[16];
    out.p9 = (int16_t)((uint16_t)c[18] << 8 | c[17]);
    out.p10 = (int8_t)c[19];
    out.p11 = (int8_t)c[20];
    return out;

}


void read_bytes(uint8_t reg, uint8_t *buf, size_t len) {

    i2c_write_blocking(i2c0, BMP_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c0, BMP_ADDR, buf, len, false);

}


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

    //#define does not write into memory so you need to make a var that holds the value of the address and use &
    uint8_t reg = REG_PRESS_DATA;

    uint8_t data[3];
    read_bytes(reg, data, 3);
    uint32_t bytes = (uint32_t) data[2] << 8*2 | data[1] << 8 | data[0];
    return bytes;
}

uint32_t read_raw_temp(void) {

    //#define does not write into memory so you need to make a var that holds the value of the address and use &
    uint8_t reg = REG_TEMP_DATA;
    uint8_t data[3];
    read_bytes(reg, data, 3);
    uint32_t bytes = (uint32_t) data[2] << 8*2 | data[1] << 8 | data[0];
    return bytes;

}



double compensate_temp(uint32_t raw_t, calib_t *c) {

    double par_t1 = c->t1 * pow(2, 8);
    double par_t2 = c->t2 / pow(2, 30);
    double par_t3 = c->t3 / pow(2, 48);

    double d1 = raw_t - par_t1;
    double d2 = d1 * par_t2;
    double temp = d2 + (d1 * d1) * par_t3;

    return temp;

}

double compensate_press(uint32_t raw_p, double t, calib_t *c) {

    //scale factory values
    double par_p1  = (c->p1 - pow(2, 14)) / pow(2, 20);
    double par_p2  = (c->p2 - pow(2, 14)) / pow(2, 29);
    double par_p3  = c->p3  / pow(2, 32);
    double par_p4  = c->p4  / pow(2, 37);
    double par_p5  = c->p5  * pow(2, 3);
    double par_p6  = c->p6  / pow(2, 6);
    double par_p7  = c->p7  / pow(2, 8);
    double par_p8  = c->p8  / pow(2, 15);
    double par_p9  = c->p9  / pow(2, 48);
    double par_p10 = c->p10 / pow(2, 48);
    double par_p11 = c->p11 / pow(2, 65);

    double p = (double)raw_p;

    //shift reading based on temp
    double out1 = par_p5 + par_p6 * t + par_p7 * t * t + par_p8 * t * t * t;
    
    double out2 = p * (par_p1 + par_p2 * t + par_p3 * t * t + par_p4 * t * t * t);
    // Small corrections
    double out3 = p * p * (par_p9 + par_p10 * t) + p * p * p * par_p11;

    return out1 + out2 + out3;

}

double pressure_to_altitude(double pa, double p0) {

    double altitude = 44330 * (1 - pow((pa/p0), 1/5.255));
    return altitude;

}

void wait_for_data(void) {

    uint8_t reg = REG_STATUS;
    uint8_t data;
    uint8_t mask = 0x60;
    while(true) {
        read_bytes(reg, &data, 1);
        sleep_ms(5);
        if((data & mask) == mask) {
            return;
        }

    }

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
    wait_for_data();
    uint8_t calib[21];
    read_bytes(REG_CALIB, calib, 21);

    calib_t cal = parse_calib(calib);

    double sum = 0;

    for(int i = 0; i < 10; i++) {

        double temp = compensate_temp(read_raw_temp(), &cal);
        double pa = compensate_press(read_raw_pressure(), temp, &cal);
        sum += pa;
        sleep_ms(50);

    }
    double p0 = sum/10;
    double smooth = 0;
    while (true) {
        
        uint32_t rt = read_raw_temp();
        uint32_t rp = read_raw_pressure();
        double t = compensate_temp(rt, &cal);
        double pa = compensate_press(rp, t, &cal);
        double altitude_m = pressure_to_altitude(pa, p0);
        if (altitude_m > 10000 || altitude_m < -10000) continue;
        smooth = smooth + 0.1 * (altitude_m - smooth);
        uint32_t now = to_ms_since_boot(get_absolute_time());
        printf("%lu,%.3f,%.1f,%.2f\n", now, smooth, pa, t);
    
        sleep_ms(100);
    }

}