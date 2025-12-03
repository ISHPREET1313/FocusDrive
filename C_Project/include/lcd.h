#ifndef LCD_H
#define LCD_H

#define rs 16
#define en 17
#define d4 18
#define d5 19
#define d6 20
#define d7 21

void lcd_write_word(uint8_t data);
void lcd_send_command(uint8_t cmd);
void lcd_init();
void lcd_send_data(uint8_t data);
void lcd_clear();
void lcd_write(uint8_t x,uint8_t y, const char * text);
void lcd_message(const char*text);

#endif