#include "pico/stdlib.h"
#include <stdio.h>
#include <stdbool.h>
#include "pico/time.h"
#include "hardware/gpio.h"
#include "include/timer.h"
#include <stdint.h>

volatile bool timer_running = false;
volatile int remaining_seconds = 0;

const int digit_pins[]={12,13,14,15};

const int SEGMENT_CODES[]={    
    0x3F,  //0
    0x06,  // 1
    0x5B,  //2
    0x4F,  //3
    0x66,  // 4
    0x6D,  // 5
    0x7D,  // 6
    0x07,  // 7
    0x7F,  // 8
    0x6F   // 9
};

void shift_out(unsigned int data){
    gpio_put(RCLK,LOW);
    for(int bit=7;bit>-1;bit--){
        gpio_put(SRCLK,LOW);
        uint8_t bit_value=(data>>bit)&1;
        gpio_put(SDI,bit_value);
        gpio_put(SRCLK,HIGH);
    }
    gpio_put(RCLK,HIGH);
}

void display_digit(uint8_t position,uint16_t digit){
    for(int d=0;d<4;d++){
        gpio_put(digit_pins[d],HIGH);
    }

    uint8_t seg_data = SEGMENT_CODES[digit];

    if(position==MIDDLE_POSITION){
        seg_data |= 0x80;
    }

    shift_out(seg_data);
    gpio_put(digit_pins[position],LOW);
    sleep_us(400);
    gpio_put(digit_pins[position],HIGH);
}

void display_number_step(uint16_t number, uint8_t multiplex_step){
    uint16_t digits[]={
        ((uint16_t)(number/1000)%10), 
        ((uint16_t)(number/100))%10, 
        ((uint16_t)(number/10))%10 , 
        number%10};
    
    display_digit(multiplex_step, digits[multiplex_step]);
}

void start_timer(int seconds){
    // Maximum displayable number is 9999. Clamp the input.
    remaining_seconds = (seconds > 9999) ? 9999 : seconds; 
    timer_running = true;
}

void timer_core1_loop(void){
    // Initialize GPIO pins for 7-segment display
    for(int i = 0; i < 4; i++){
        gpio_init(digit_pins[i]);
        gpio_set_dir(digit_pins[i], GPIO_OUT);
        gpio_put(digit_pins[i], HIGH); // Start all off
    }
    gpio_init(SDI);
    gpio_set_dir(SDI, GPIO_OUT);
    gpio_init(RCLK);
    gpio_set_dir(RCLK, GPIO_OUT);
    gpio_init(SRCLK);
    gpio_set_dir(SRCLK, GPIO_OUT);

    uint64_t last_second_update = time_us_64();
    uint64_t last_multiplex_update = time_us_64();
    uint8_t current_digit = 0;

    while(1){
        uint64_t now = time_us_64();
        
        
        if (timer_running) {
            if (now - last_second_update >= 1000000){ 
                last_second_update += 1000000;
                if(remaining_seconds > 0) {
                    remaining_seconds--;
                } else {
                    timer_running = false; 
                }
            }
        } else {
            
            last_second_update = now;
        }
        
        const uint64_t multiplex_delay_us = 4000;
        if(now - last_multiplex_update >= multiplex_delay_us){
            last_multiplex_update += multiplex_delay_us;

            uint16_t total_seconds = (uint16_t)remaining_seconds;
            
            
            uint16_t minutes = total_seconds / 60;
            uint16_t seconds = total_seconds % 60;

            
            uint16_t digits[4];

            
            if (!timer_running && total_seconds == 0) {
                digits[0] = 0; 
                digits[1] = 0; 
                digits[2] = 0; 
                digits[3] = 0;
            } else {
                
                digits[0] = (minutes / 10) % 10;
                digits[1] = minutes % 10;
                
                
                digits[2] = (seconds / 10) % 10;
                digits[3] = seconds % 10;
            }
            
            
            display_digit(current_digit, digits[current_digit]);
            

            current_digit = (current_digit + 1) % 4;
        }
        
        tight_loop_contents();
    }
}