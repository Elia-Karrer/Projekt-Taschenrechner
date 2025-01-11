// Ausgabe von Integern: Nur im 16-bit Raum
// Änderung: "%ld" statt "%u"

// Ausgabe von Floats: "?"
// Änderung: dtostrf() statt sprintf(), weil sprintf auf microcontrollern kein "%f" unterstützt

// Memory probleme


#include <Arduino.h>
#include "keyboard.h"
#include "display.h"
#include "calculator.h"


char input[INPUT_BUFSIZE];
char output[OUTPUT_BUFSIZE];
char variable[OUTPUT_BUFSIZE];
element elements[ELEMENTS_BUFSIZE];


void input_into_string(char* input, const uint32_t max_len)
{
    uint32_t i;
    char c;

    i = 0;
    while(1)
    {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Input = ");
        display.print(input);
        display.display();

        c = keyboard.get_key();

        if(c == '=') // Enter
        {
            input[i] = '\0';
            return;
        }

        if(c == 'D') // DEL
        {
            if(i >= 1)
            {
                input[i-1] = '\0';
                i--;
            }

            continue;
        }

        if(c == '_') // Unbelegte Taste
            continue;

        if(c == 'S')    // Cannot put 'STO' into input string
            continue;

        if(i <= (max_len-2))
        {
            input[i] = c;
            input[i+1] = '\0';
            i++;
        }
    }
}


void setup(void)
{
    Serial.begin(9600);

    keyboard.begin();
    init_display();
}


void loop(void)
{
    uint8_t err;
    uint32_t i;

    // Init variable as undefined (Empty string "")
    variable[0] = '\0';

    while(1)
    {
        // Clear everything
        // Can be removed to be more efficient, but is safer
        for(i = 0; i < INPUT_BUFSIZE; i++)
            input[i] = '\0';
        for(i = 0; i < OUTPUT_BUFSIZE; i++)
            output[i] = '\0';
        for(i = 0; i < ELEMENTS_BUFSIZE; i++)
            elements[i].type = 'E';

        // Eingabe
        input_into_string(input, INPUT_BUFSIZE);

        // Calculate
        err = calculate_output_from_input(input, output, variable, elements);
        
        display.clearDisplay();
        display.setCursor(0, 0);
        if(err == 2)    // Just do nothing when nothing is input
        {
            display.clearDisplay();
            display.display();
            continue;
        }
        else if(err)
        {
            display.print("Error\n");
            display.print(output);
        }
        else
        {
            display.print("Result = \"");
            display.print(output);
            display.print("\"");
        }
        display.display();
        
        // Eingabe nach Ergebnis
        uint8_t c = keyboard.get_key();

        if(!err) // Do not store errors
            if(c == 'S') // store result
            {
                for(i = 0; i <= str_len(output); i++) // Copy delimiter too
                    variable[i] = output[i];
            }

        display.clearDisplay();
        display.display();
    }
}
