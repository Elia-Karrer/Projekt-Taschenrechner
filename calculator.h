//#define CONSOLE_AVAILABLE

#ifndef CALCULATOR_H
#define CALCULATOR_H


#include <Arduino.h>

#define IS_NUM(X)       (((X) >= '0') && (((X) <= '9')))
#define IS_BRACE(X)     (((X) == '(') || ((X) == ')'))
#define IS_OPERATOR(X)  (((X) == '+') || ((X) == '-') || ((X) == '*') || ((X) == '/'))

#define DECIMAL_SEPARATOR       '.'
#define VARIABLE_CHAR           'R'

#define INPUT_BUFSIZE           40
#define OUTPUT_BUFSIZE          40
#define ELEMENTS_BUFSIZE        ((INPUT_BUFSIZE / 2) + 5)       // +5 for wrapping "()" etc.


typedef union {
    float f;
    int32_t i;
} int_or_float;


typedef struct {
    char type;      // First pass: "N+-*/()."  Second Pass: "IF+-*/()"

	char* start;
	uint8_t len;

    int_or_float var;
} element;


uint32_t    parse_int                   (const char* const start, uint8_t len);
float       parse_float                 (const char* const start_left, uint8_t len_left, const char* const start_right, uint8_t len_right);
uint32_t    count_elements_from_str     (char* s);
uint8_t     str_to_elements             (char* s, element* elements, const uint32_t maximum_len);
void        parse_numbers               (element* elements);
element     calculate_innerbrace        (element* elements, uint32_t start, uint32_t end);
element     calculate_elements          (element* elements);
uint8_t     is_valid_syntax             (char c1, char c2);
uint8_t     syntax_check                (element* elements);
uint32_t    str_len                     (char* s);
uint8_t     preprocess                  (char* input, const uint32_t bufsize, char* variable);
uint8_t     calculate_output_from_input (char* input, char* output, char* variable, element* elements);
#ifdef CONSOLE_AVAILABLE
void        print_elements              (element* elements);
void        print_elements_as_string    (element* elements);
#endif


#endif
