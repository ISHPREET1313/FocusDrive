#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "include/buzzer.h"


void buz_init() {
    gpio_init(BUZ_PIN);
    gpio_set_dir(BUZ_PIN, GPIO_OUT);
    gpio_put(BUZ_PIN, 0); 
}

void buzzer() {
    for(int i=0;i<10;i++){
        gpio_put(BUZ_PIN,1);
        sleep_ms(50);
        gpio_put(BUZ_PIN,0);
        sleep_ms(50);
    }
}