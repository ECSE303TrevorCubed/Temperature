#ifndef TEMPER_LCD_H_
#define TEMPER_LCD_H_

#include <stdbool.h>
#include <stdint.h>

// Follows the Hitachi HD44780U Datasheet and
// https://github.com/arduino-libraries/LiquidCrystal/blob/master/src/LiquidCrystal.cpp
typedef struct {
  int fd;
} lcd_t;

typedef enum {
  ROW_TOP,
  ROW_BOTTOM,
} lcd_row_t;

bool lcd_init(lcd_t *lcd);
void lcd_puts(lcd_t lcd, const char *str);
void lcd_printf(lcd_t lcd, const char *fmt, ...);
void lcd_set_cursor(lcd_t lcd, uint8_t col, lcd_row_t row);
void lcd_clear(lcd_t lcd);
void lcd_home(lcd_t lcd);

#endif
