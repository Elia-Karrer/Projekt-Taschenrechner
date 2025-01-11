// SCL = A5
// SDA = A4


#ifndef DISPLAY_H
#define DISPLAY_H


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_WIDTH  128
#define OLED_HEIGHT 32

void init_display(void);


extern Adafruit_SSD1306 display;

#endif