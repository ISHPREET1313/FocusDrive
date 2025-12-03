#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "include/lcd.h"
#include "include/button.h"
#include "include/buzzer.h"
#include "include/timer.h"

#define LCD_MAX_LEN 15
#define BUF_SIZE 64

extern volatile bool timer_running;
extern volatile int remaining_seconds;

// Read one line from USB (
int usb_read_line(char *buffer, int max_len)
{
    int index = 0;
    while (true)
    {
        int c = getchar_timeout_us(100000); 
        if (c < 0)
            continue; 

        if (c == '\r')
            continue;
        if (c == '\n')
        {
            buffer[index] = '\0';
            return index;
        }

        if (index < max_len - 1)
            buffer[index++] = (char)c;
        else
            index = 0; 
    }
}

int main()
{
    stdio_init_all();
    lcd_init();
    but_init();
    buz_init();
    sleep_ms(50);
   
    multicore_launch_core1(timer_core1_loop);
    
    while (1)
    {
        lcd_clear();
        lcd_write(0, 0, "Ready for Task");

        while (!stdio_usb_connected())
            sleep_ms(100);
        printf("USB Ready!\n");
        char line[BUF_SIZE];
        uint16_t timer;
        char name[LCD_MAX_LEN];
        int min, sec;
        while (true)
        {
            usb_read_line(line, BUF_SIZE);
            if (sscanf(line, "%14s %d:%d", name, &min, &sec) == 3)
            {
                timer = min * 60 + sec;
                printf("Recieved %s (%02d:%02d)\n", name, min, sec);
                printf("Starting Timer");
                break;
            }
            else
            {
                printf("Invalid task format\n");
            }
        }
        lcd_clear();
        char task[LCD_MAX_LEN];
        sprintf(task, "Got Task %s", name);
        lcd_write(0, 0, task);
        char timer_number[LCD_MAX_LEN];
        sprintf(timer_number, "%02d:%02d", min, sec);
        lcd_write(0, 1, timer_number);
        sleep_ms(3000);
        for (int i = 3; i >= 0; i--)
        {
            lcd_clear();
            lcd_write(0, 0, "Starting Task");
            char str[LCD_MAX_LEN];
            sprintf(str,"   in %d",i);
            lcd_write(0, 1, str);
            sleep_ms(1000);
        }
        start_timer(timer);
        lcd_clear();
        lcd_write(0, 0, task);
        int bar_width = 16;
        int progress_fill_len = bar_width - 2;
        const int total_time_for_progress = timer;
        while (timer_running || remaining_seconds > 0)
        {
            
            float elapsed_time = (float)(timer - remaining_seconds);
            float total_time = (float)timer;
            
            
            int progress = 0;
            if (total_time > 0) {
                progress = (int)((elapsed_time / total_time) * progress_fill_len);
            }
            
            
            if (progress > progress_fill_len) {
                progress = progress_fill_len;
            }
            
            
            char bar_str[LCD_MAX_LEN + 1];
            bar_str[0] = '[';
            for (int i = 0; i < progress_fill_len; i++)
            {
                bar_str[i + 1] = (i < progress) ? '#' : '-';
            }
            bar_str[bar_width - 1] = ']';
            bar_str[bar_width] = '\0';
            
            lcd_write(0, 1, bar_str);
            
            
            sleep_ms(100); 
            if (!timer_running && remaining_seconds <= 0) {
                break;
            }
        }
        lcd_write(0, 1, "[##############]");
        lcd_clear();
        srand(time(0));
        int exit = 0;
        while (exit < 4)
        {
            lcd_write(0, 0, "     Press     ");
            int random_in_range = (rand() % 4) + 1;
            int target_pin = 0;

            
            if (random_in_range == 1)
            {
                lcd_write(0, 1, "  RED Button  ");
                target_pin = RED;
            }
            else if (random_in_range == 2)
            {
                lcd_write(0, 1, "  BLUE Button  ");
                target_pin = BLUE;
            }
            else if (random_in_range == 3)
            {
                lcd_write(0, 1, " YELLOW Button ");
                target_pin = YELLOW;
            }
            else 
            {
                lcd_write(0, 1, "  WHITE Button ");
                target_pin = WHITE;
            }

            
            while (but_output(target_pin) != 0)
            {
                buzzer(); 
                sleep_ms(10);
            }
            
            
            exit++;
            lcd_clear();
            sleep_ms(200); 
        }
        lcd_clear();
        lcd_write(0, 0, "Task FINISHED");
        lcd_write(0, 1, "  CONGRATS!!!");
        sleep_ms(3000);
    }
}
