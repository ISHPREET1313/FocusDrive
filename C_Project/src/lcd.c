#include "pico/stdlib.h"
#include <stdio.h>
#include "pico/time.h"
#include <string.h>
#include "hardware/gpio.h"
#include "include/lcd.h"
#include <stdint.h>

const int lcd_pins[]={16,17,18,19,20,21};

void lcd_write_word(uint8_t data){
    gpio_put(d4,(data>>4)&1);
    gpio_put(d5,(data>>5)&1);
    gpio_put(d6,(data>>6)&1);
    gpio_put(d7,(data>>7)&1);

    gpio_put(en,1);
    sleep_us(50);
    gpio_put(en,0);
    sleep_us(50);
}


void lcd_send_command(uint8_t cmd){
    gpio_put(rs,0);

    lcd_write_word(cmd & 0xF0);
    lcd_write_word((cmd<<4)&0xF0);
    sleep_ms(2);

}


void lcd_init(){
    for(int i=0;i<6;i++){
        gpio_init(lcd_pins[i]);
        gpio_set_dir(lcd_pins[i],GPIO_OUT);
    }

    sleep_ms(20);

    lcd_send_command(0x33);
    sleep_ms(5);
    lcd_send_command(0x32);
    sleep_ms(5);
    lcd_send_command(0x28);
    sleep_ms(5);
    lcd_send_command(0x0C);
    sleep_ms(5);
    lcd_send_command(0x01);
    sleep_ms(5);
}


void lcd_send_data(uint8_t data){
    gpio_put(rs,1);

    lcd_write_word(data & 0xF0);
    lcd_write_word((data<<4)& 0xF0);

    sleep_us(50);
}

void lcd_clear(){
    lcd_send_command(0x01);
    sleep_ms(2);
}

void lcd_write(uint8_t x,uint8_t y, const char * text){
    if (x<0) x=0;
    if (x>15) x=15;
    if (y<0) y=0;
    if (y>1) y=1;

    uint8_t addr = 0x80 + 0x40*y +x;
    lcd_send_command(addr);

    for(int i=0;i<strlen(text);i++){
        lcd_send_data((int)text[i]);
    }
}

void lcd_message(const char*text){
    for(int i=0;i<strlen(text);i++){
        if(text[i]=='\n')
            lcd_send_command(0xC0);
        else
            lcd_send_data((int)text[i]);
    }
}
