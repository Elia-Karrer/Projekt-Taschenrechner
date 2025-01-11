#ifndef KEYBOARD_H
#define KEYBOARD_H


#include <Arduino.h>


class Keyboard
{
private:
    // NOTE: When changing Pins: Be aware, that the x of PORTx/DDRx also has to be changed in read() and read_shift()
    const uint8_t mask_x[4]              = {1<<PD2, 1<<PD3, 1<<PD4, 1<<PD5};
    const uint8_t mask_y[5]              = {1<<PB5, 1<<PB4, 1<<PB3, 1<<PB2, 1<<PB1};
    const uint8_t mask_shift             = (1 << PD6);
    const char    keymap_primary[20+1]   = "789/456*123-0.D+()S=";
    const char    keymap_secondary[20+1] = "__________________R_";

    uint8_t read(void);
    inline bool read_shift(void);

public:
    void begin(void);
    char get_key(void);
};


extern Keyboard keyboard;


#endif
