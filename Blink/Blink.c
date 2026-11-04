#include <stdio.h>
#include "pico/stdlib.h"

void setup(int pin) {

    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);


}

int main()
{
    stdio_init_all();

    int pin = PICO_DEFAULT_LED_PIN;
    setup(pin);

    bool pinval = 0;
    char status[2][4] = {"OFF", "ON"};

    while(true) {

        gpio_put(pin, pinval);
        printf("LED is %s\n", status[pinval]);
        pinval = !pinval;
        sleep_ms(500);


    }


}
