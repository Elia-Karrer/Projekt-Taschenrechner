#include "keyboard.h"


Keyboard keyboard;


void Keyboard::begin(void)
{
    uint8_t i;

    for(i = 0; i < 4; i++)
        DDRD |= mask_x[i];     // OUTPUT

    for(i = 0; i < 5; i++)
    {
        DDRB  &= ~(mask_y[i]);  // INPUT
        PORTB |= mask_y[i];     // Pullup = ON
    }

    PORTD |= mask_shift;
    DDRD |= mask_shift;
}


uint8_t Keyboard::read(void)
{
    uint8_t x, y;

    for(x = 0; x < 4; x++)
    {
        PORTD |= mask_x[0] | mask_x[1] | mask_x[2] | mask_x[3];     // All off
        PORTD &= ~(mask_x[x]);                                      // Activate column

        for(y = 0; y < 5; y++)
            if((PINB & mask_y[y]) == 0)     // Found pressed key
            {
                uint8_t index = y*4 + x;

                PORTD |= mask_x[0] | mask_x[1] | mask_x[2] | mask_x[3];
                return index;
            }
    }

    PORTD |= mask_x[0] | mask_x[1] | mask_x[2] | mask_x[3];
    return 0xFF;
}


inline bool Keyboard::read_shift(void)
{
    return !(PIND & mask_shift);
}


// Waits for release of key
char Keyboard::get_key(void)
{
    uint8_t key;
    bool secondary;

    // Wait for keypress
    do
        key = read();
    while(key == 0xFF);
    secondary = read_shift();

    // Wait for release
    while(read() == key);

    // Limit maximum pressing frequency AND debounce
    delay(50);

    return secondary ? keymap_secondary[key] : keymap_primary[key];
}
