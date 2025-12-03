#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "include/button.h"

const int but_pins[]={28,27,26,22};

void but_init(){
    for(int i=0;i<4;i++){
        gpio_init(but_pins[i]);
        gpio_set_dir(but_pins[i],GPIO_IN);
        gpio_pull_up(but_pins[i]);
    }
}

int but_output(int Pin){
    return gpio_get(Pin);
}