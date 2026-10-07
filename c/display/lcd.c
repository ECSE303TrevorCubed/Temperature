#include "lcd.h"

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>

// Specific to the RPi and our LCD
// https://www.crystalfontz.com/controllers/datasheet-viewer.php?id=97
#define PIN_CMD 0       // Command register
#define PIN_RS (1 << 0) // Data register
#define PIN_EN (1 << 2) // Enable
#define I2C_DEV "/dev/i2c-1"
#define I2C_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2
#define BACKLIGHT_BIT (1 << 3)
#define TOP_ROW_ADDR 0x00    // From datasheet
#define BOTTOM_ROW_ADDR 0x40 // From datasheet

// From
// https://github.com/arduino-libraries/LiquidCrystal/blob/master/src/LiquidCrystal.h#L8C1-L43C25
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_SETDDRAMADDR 0x80
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSOROFF 0x00
#define LCD_BLINKOFF 0x00
#define LCD_FUNCTIONSET 0x20
#define LCD_ENTRYSHIFTDECREMENT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_5x8DOTS 0x00
#define LCD_2LINE 0x08

// Writes the upper 4 bits of the provided byte
// https://github.com/arduino-libraries/LiquidCrystal/blob/master/src/LiquidCrystal.cpp#L303
static void lcd_write_nibble(int fd, uint8_t nibble, uint8_t mode) {
  uint8_t byte = (nibble & 0xF0) | mode | BACKLIGHT_BIT;
  wiringPiI2CWrite(fd, byte | PIN_EN);
  delayMicroseconds(1); // Satisfies 500 ns min to hold high
  wiringPiI2CWrite(fd, byte & ~PIN_EN);
  delayMicroseconds(50); // Commands need above 37 us to settle
}

// Writes the entire byte, always with 4-bit selection
// https://github.com/arduino-libraries/LiquidCrystal/blob/master/src/LiquidCrystal.cpp#L287
static void lcd_send(int fd, uint8_t value, uint8_t mode) {
  lcd_write_nibble(fd, value & 0xF0, mode);
  lcd_write_nibble(fd, (value << 4) & 0xF0, mode);
}

static void lcd_command(int fd, uint8_t cmd) { lcd_send(fd, cmd, PIN_CMD); }

static void lcd_write(int fd, uint8_t ch) { lcd_send(fd, ch, PIN_RS); }

void lcd_printf(lcd_t lcd, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  char buf[512]; // Should be long enough
  vsnprintf(buf, sizeof(buf), fmt, args);
  lcd_puts(lcd, buf);
  va_end(args);
}

bool lcd_init(lcd_t *lcd) {
  assert(lcd && "LCD pointer must not be null");
  lcd->fd = wiringPiI2CSetupInterface(I2C_DEV, I2C_ADDR);
  if (lcd->fd < 0)
    return false;

  // Power-up delay of >40ms needed
  delay(50);

  // 4-bit reset seq following:
  // https://github.com/arduino-libraries/LiquidCrystal/blob/master/src/LiquidCrystal.cpp#L120C5-L133C23
  // 0x30 instead of 0x03 since write is top bits
  lcd_write_nibble(lcd->fd, 0x30, 0); // first try
  delayMicroseconds(4500);
  lcd_write_nibble(lcd->fd, 0x30, 0); // second try
  delayMicroseconds(4500);
  lcd_write_nibble(lcd->fd, 0x30, 0); // third try
  delayMicroseconds(150);
  lcd_write_nibble(lcd->fd, 0x20, 0); // finally, set 4-bit interface
  delayMicroseconds(150);

  // 4-bit mode, 2-line, 5x8 dots
  lcd_command(lcd->fd, LCD_FUNCTIONSET | LCD_2LINE | LCD_5x8DOTS);

  // Display ON, cursor OFF, blink OFF
  lcd_command(lcd->fd, LCD_ENTRYMODESET | LCD_DISPLAYCONTROL | LCD_CURSOROFF |
                           LCD_BLINKOFF);
  // Clear display
  lcd_clear(*lcd);
  // Entry mode: Increment cursor (left to right)
  lcd_command(lcd->fd,
              LCD_ENTRYMODESET | LCD_ENTRYSHIFTDECREMENT | LCD_ENTRYLEFT);

  return true;
}

void lcd_puts(lcd_t lcd, const char *str) {
  while (*str) {
    lcd_write(lcd.fd, (uint8_t)*str++);
  }
}

void lcd_set_cursor(lcd_t lcd, lcd_row_t row, uint8_t col) {
  if (row != ROW_TOP &&
      row != ROW_BOTTOM) // me when c's enums are always exhaustive
    return;
  const uint8_t row_offset = row == ROW_TOP ? TOP_ROW_ADDR : BOTTOM_ROW_ADDR;
  lcd_command(lcd.fd, LCD_SETDDRAMADDR | (col + row_offset));
}

// Might take a while according to:
// https://github.com/arduino-libraries/LiquidCrystal/blob/master/src/LiquidCrystal.cpp#L179
void lcd_clear(lcd_t lcd) {
  lcd_command(lcd.fd, LCD_CLEARDISPLAY);
  delayMicroseconds(2000);
}

// Apparently this takes a while (1.52 ms per spec)
void lcd_home(lcd_t lcd) {
  lcd_command(lcd.fd, LCD_RETURNHOME);
  delayMicroseconds(2000);
}
