#ifndef TIMER_H
#define TIMER_H

#define MIDDLE_POSITION 1
#define SDI 11
#define RCLK 10
#define SRCLK 9
#define LOW 0
#define HIGH 1


extern volatile bool timer_running;
extern volatile int remaining_seconds;

void shift_out(unsigned int data);
void display_digit(uint8_t position,uint16_t digit);
void display_number_step(uint16_t number, uint8_t multiplex_step);
void start_timer(int seconds);

void timer_core1_loop(void);

#endif