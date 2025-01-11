#include "calculator.h"


/// TODO: preprocess(): Stern zwischen )( und N( und )N setzen



uint32_t parse_int(const char* const start, uint8_t len)
{
    uint32_t output, i, mult;

    // Exceeded max. value
    if(len > 9)
        return 999999999;

    mult = 1;
    output = 0;
    for(i = 0; i < len; i++)
    {
        char current_char = start[(len-1)-i];
        uint8_t current_digit = current_char - '0';

        output += current_digit * mult;
        mult *= 10;
    }

    return output;
}


float parse_float(const char* const start_left, uint8_t len_left, const char* const start_right, uint8_t len_right)
{
    float left, right;
    const float mult[] = {1, 10, 100, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9};

    // Exceeded max. value
    if(len_left > 9)
        return 999999999.999999999f;

    // Only first 9 decimals are accepted
    if(len_right > 9)
        len_right = 9;

    left = (float) parse_int(start_left, len_left);
    right = ((float) parse_int(start_right, len_right)) / mult[len_right];

    return left + right;
}


// Counts +-*/(). and Numbers (type=N)
// "+3333" is counted as 2 elements
uint32_t count_elements_from_str(char* s)
{
    uint16_t chrcnt = 0;
    uint16_t elem_cnt = 0;


    while(1)
    {
        if(IS_OPERATOR(s[chrcnt]) || IS_BRACE(s[chrcnt]) || (s[chrcnt] == DECIMAL_SEPARATOR))
        {
            chrcnt++;
            elem_cnt++;
            continue;
        }

        if(IS_NUM(s[chrcnt]))
        {
            while(IS_NUM(s[chrcnt]))
                chrcnt++;

            elem_cnt++;
            continue;
        }

        if(s[chrcnt] == '\0')
            break;

        // Unknown character
        return -1;
    }

    return elem_cnt;
}


// Data from String is written into Elements
// One-Character elements are written directly
// Numbers are only written as start positon (char*) and length (uint8)
// 1 = Error
// 0 = Worked
uint8_t str_to_elements(char* s, element* elements, const uint32_t maximum_len)
{
    uint32_t chrcnt, elem_cnt, counted_len;

    // Check if given element-array is big enough, else return error
    counted_len = count_elements_from_str(s);

    if(counted_len == -1UL)       // Unknown characters
        return 1;
    if(counted_len > maximum_len)
        return 1;

    // Set terminating element
    elements[counted_len].type = 'E';

    chrcnt = 0;
    elem_cnt = 0;
    while(1)
    {
        if(IS_OPERATOR(s[chrcnt]) ||  IS_BRACE(s[chrcnt]) || s[chrcnt] == DECIMAL_SEPARATOR)
        {
            elements[elem_cnt].type = s[chrcnt];

            chrcnt++;
            elem_cnt++;
            continue;
        }

        if(IS_NUM(s[chrcnt]))
        {
            elements[elem_cnt].type = 'N';
            elements[elem_cnt].start = &s[chrcnt];

            while(IS_NUM(s[chrcnt]))
                chrcnt++;

            elements[elem_cnt].len = &s[chrcnt] - elements[elem_cnt].start;

            // Number too long
            if(elements[elem_cnt].len > 9)
                return 1;

            elem_cnt++;
            continue;
        }

        if(s[chrcnt] == '\0')
            break;

        // Is invalid character
        return 1;
    }

    return 0;
}


// Zahlen werden geparst
// Converts N.N to F
// Converts N to I
void parse_numbers(element* elements)
{
    uint32_t i, j;

    // Alle ints und floats parsen und direkt reinschreiben
    i = 0;
    while(1)
    {
        // Number
        if(elements[i].type == 'N')
        {
            if(elements[i+1].type == DECIMAL_SEPARATOR) // FLOAT
            {
                elements[i].type = 'F';
                elements[i].var.f = parse_float(elements[i].start, elements[i].len, elements[i+2].start, elements[i+2].len); // +2 weil "." dawzischen ist

                // 0 = "."
                // j+1 = zweite zahl
                // J+2 = alles nach N.N
                j = i+1;
                while(1)
                {
                    /// OPTIMIERUNG HIER WÄRE: NUR SINNVOLLE WERTE SHIFTEN (I und F nicht, weil es grantiert noch nicht geparst wurde)
                    elements[j] = elements[j+2];

                    // Beim ende wird davor noch ende gesetzt
                    if(elements[j+2].type == 'E')
                        break;

                    j++;
                }

                // i++, because new data was shifted
                i++;
                continue;
            }
            else // INT
            {
                elements[i].type = 'I';
                elements[i].var.i = parse_int(elements[i].start, elements[i].len);

                i++;
                continue;
            }
        }

        if(IS_BRACE(elements[i].type) || IS_OPERATOR(elements[i].type))
        {
            i++;
            continue;
        }

        if(elements[i].type == 'E')
        {
            break;
        }
    }
}


// start: (
// end:   )
// Every operator HAS to be sorrounded by numbers: i*i, f+f, ...
// i+i*i wird zu: iZZZZ und Ergebnis wird retourniert
element calculate_innerbrace(element* elements, uint32_t start, uint32_t end)
{
    uint32_t i, j;

    // Check if only 1 element in brace
    if(start-end == 2)
        return elements[start+1];

    // If first element after brace is -, negate first number
    if(elements[start+1].type == '-')
    {
        if(elements[start+2].type == 'I' || elements[start+2].type == 'F')
        {
            if(elements[start+2].type == 'I')
                elements[start+2].var.i = -elements[start+2].var.i;
            if(elements[start+2].type == 'F')
                elements[start+2].var.f = -elements[start+2].var.f;

            // Copy negated number to position of minus
            elements[start+1] = elements[start+2];

            // Shift everything by 1 Position
            i = start+2;
            while(1)
            {
                elements[i] = elements[i+1];

                // Stop shifting when end was copied
                if(i == end)
                    break;

                i++;
            }
            end--;
        }
    }

    // Do multiplications and divisions
    i = start + 1;
    while(1)
    {
        if(i == end)
            break;

        if(elements[i].type == '*' || elements[i].type == '/')
        {
            /// Annahme: [i-1] ist I oder F !!!
            /// Annahme: [i+1] ist I oder F !!!

            // Zero-division-check
            if(elements[i].type == '/')
            {
                uint8_t is_nulldivision = 0;

                if(elements[i+1].type == 'I')
                    if(elements[i+1].var.i == 0)
                        is_nulldivision = 1;

                if(elements[i+1].type == 'F')
                    if(elements[i+1].var.f == 0)
                        is_nulldivision = 1;

                if(is_nulldivision)
                    return (element){.type='E'};
            }


            // Integer operation
            if(elements[i-1].type == 'I' && elements[i+1].type == 'I')
            {
                // Calculation i*i
                if(elements[i].type == '*')
                    elements[i-1].var.i = elements[i-1].var.i * elements[i+1].var.i;

                // Calculate i/i
                if(elements[i].type == '/')
                {
                    // Wenn Restwert, berechne als float
                    if((elements[i-1].var.i % elements[i+1].var.i) != 0)
                    {
                        elements[i-1].type = 'F';
                        elements[i-1].var.f = ((float) elements[i-1].var.i) / ((float) elements[i+1].var.i);
                    }

                    // Wenn Restwertlose Ganzzahldivision, berechne als int
                    else
                        elements[i-1].var.i = elements[i-1].var.i / elements[i+1].var.i;
                }

                if(elements[i-1].var.i > 999999999 || elements[i-1].var.i < -999999999)
                    return (element){.type='E'};
            }

            // Float operation
            else if(elements[i-1].type == 'F' || elements[i+1].type == 'F')
            {
                // Casting, if integers are there too
                if(elements[i-1].type == 'I')
                {
                    elements[i-1].type = 'F';
                    elements[i-1].var.f = (float) elements[i-1].var.i;
                }
                if(elements[i+1].type == 'I')
                {
                    elements[i+1].type = 'F';
                    elements[i+1].var.f = (float) elements[i+1].var.i;
                }

                // Calculation f*f or f/f
                if(elements[i].type == '*')
                    elements[i-1].var.f = elements[i-1].var.f * elements[i+1].var.f;
                if(elements[i].type == '/')
                    elements[i-1].var.f = elements[i-1].var.f / elements[i+1].var.f;

                if(elements[i-1].var.f >= 1000000000.0F || elements[i-1].var.f < -1000000000.0F)
                    return (element){.type='E'};
            }


            // i*i wird zu i, daher "*i" löschen und daten dahinter nach-shiften
            for(j = i; j < (end-2); j++)
            {
                elements[j].type = elements[j+2].type;
                elements[j].var.i = elements[j+2].var.i;
                elements[j].var.f = elements[j+2].var.f;
            }

            // Freigewordene Elemente (Am Ende) als unnötig deklarieren
            // [Werden aber sowieso bei der Klammerauflösung weggeshiftet]
            /// OPTIMIERUNG: EINFACH NICHT Z und Z SCHREIBEN, nur end um 2 verringern, weil leere werden SOWIESO WEGGESHIFTET
            elements[end-2].type = 'Z';
            elements[end-1].type = 'Z';
            end -= 2;

            #ifdef CONSOLE_AVAILABLE
            printf("> "); print_elements_as_string(elements);
            #endif

            // Start all over again and search multiplications
            i = start+1;
            continue;
        }

        i++;
    }

    // Do additions and subtractions
    i = start + 1;
    while(1)
    {
        if(i == end)
            break;

        if(elements[i].type == '+' || elements[i].type == '-')
        {
            /// Annahme: [i-1] ist I oder F !!!
            /// Annahme: [i+1] ist I oder F !!!

            // Integer operation
            if(elements[i-1].type == 'I' && elements[i+1].type == 'I')
            {
                // Calculation i*i or i/i
                if(elements[i].type == '+')
                    elements[i-1].var.i = elements[i-1].var.i + elements[i+1].var.i;
                if(elements[i].type == '-')
                    elements[i-1].var.i = elements[i-1].var.i - elements[i+1].var.i;

                if(elements[i-1].var.i > 999999999 || elements[i-1].var.i < -999999999)
                    return (element){.type='E'};
            }

            // Float operation
            else if(elements[i-1].type == 'F' || elements[i+1].type == 'F')
            {
                // Casting, if integers are there too
                if(elements[i-1].type == 'I')
                {
                    elements[i-1].type = 'F';
                    elements[i-1].var.f = (float) elements[i-1].var.i;
                }
                if(elements[i+1].type == 'I')
                {
                    elements[i+1].type = 'F';
                    elements[i+1].var.f = (float) elements[i+1].var.i;
                }

                // Calculation f*f or f/f
                if(elements[i].type == '+')
                    elements[i-1].var.f = elements[i-1].var.f + elements[i+1].var.f;
                if(elements[i].type == '-')
                    elements[i-1].var.f = elements[i-1].var.f - elements[i+1].var.f;

                if(elements[i-1].var.f >= 1000000000.0F || elements[i-1].var.f < -1000000000.0F)
                    return (element){.type='E'};
            }


            // i*i wird zu i, daher "*i" löschen und daten dahinter nach-shiften
            for(j = i; j < (end-2); j++)
                elements[j] = elements[j+2];

            // Freigewordene Elemente (Am Ende) als unnötig deklarieren
            // [Werden aber sowieso bei der Klammerauflösung weggeshiftet]
            /// OPTIMIERUNG: EINFACH NICHT MACHEN, WIRD SOWIESO WEGGESHIFTET
            elements[end-2].type = 'Z';
            elements[end-1].type = 'Z';
            end -= 2;

            #ifdef CONSOLE_AVAILABLE
            printf("> "); print_elements_as_string(elements);
            #endif

            // Start all over again and search multiplications
            i = start+1;
            continue;
        }

        i++;
    }

    // return result (element after first brace)
    return elements[start+1];
}


// Klammern werden auch überprüft
// Rechnet in richtiger reihenfolge die Klammern, innerhalb der klammer wird mit calculate_innerbrace() gelöst
// Mindestens eine klammerebene MUSS existieren
element calculate_elements(element* elements)
{
    uint32_t i, len;
    int current_brace_level, max_brace_level;
    uint32_t search_for;


    // count len, check for floats, count highest brace level
    i = 0;
    len = 0;
    current_brace_level = 0;
    max_brace_level = 0;
    while(1)
    {
        if(IS_BRACE(elements[i].type))
        {
            if(elements[i].type == '(')
                current_brace_level++;

            if(elements[i].type == ')')
                current_brace_level--;

            if(current_brace_level > max_brace_level)
                max_brace_level = current_brace_level;

            // Checke braces nur hier in dem Abschnitt wenn sich was ändert
            // Falls ")))" oder "()" steht
            if(current_brace_level < 0 || (elements[i].type == '(' && elements[i+1].type == ')'))
                return (element){.type = 'E'};
        }

        if(elements[i].type == 'E')
            break;
        len++;
        i++;
    }

    // Check if some braces were not closed "(()"
    if(current_brace_level != 0)
        return (element){.type = 'E'};

    // Iterate over all elements, store brace level
    search_for = max_brace_level;
    current_brace_level = 0;
    i = 0;
    while(1)
    {

        if(elements[i].type == '(')
            current_brace_level++;
        if(elements[i].type == ')')
            current_brace_level--;

        if(elements[i].type == 'E')
        {
            if(current_brace_level == 0)
                break;

            search_for--;
            // Go back to start and search for braces in new wanted level
            i = 0;
            current_brace_level = 0;
        }

        if(current_brace_level == ((int) search_for))
        {
            uint32_t open_brace_pos = i;
            uint32_t close_brace_pos;

            while(1)
            {
                i++;

                // Muss ')' sein, weil von oberstem brace level heruntergezählt wird
                if(elements[i].type == ')')
                {
                    close_brace_pos = i;
                    break;
                }
            }

            // Calculate everything inner brace: (i*i+f) wird zu (fZZZZ)
            // Result kommt direkt auf stelle erster klammer: ffZZZZ)
            elements[open_brace_pos] = calculate_innerbrace(elements, open_brace_pos, close_brace_pos);

            // Wenn result keine zahl, retourniere den error
            if(!(elements[open_brace_pos].type == 'I' || elements[open_brace_pos].type == 'F'))
                return (element){.type = 'E'};

            // Shift away empty elements (Use brace positions as iterators)
            while(1)
            {
                open_brace_pos++;
                close_brace_pos++;

                elements[open_brace_pos] = elements[close_brace_pos];

                // Stop shifting when end of elements is reached
                if(elements[close_brace_pos].type == 'E')
                    break;
            }

            #ifdef CONSOLE_AVAILABLE
            print_elements_as_string(elements);
            #endif

            // Go back to start and search for braces in wanted level
            i = 0;
            current_brace_level = 0;
        }

        i++;
    }

    return elements[0];
}


// 1 = VALID
// 0 = INVALID
uint8_t is_valid_syntax(char c1, char c2)
{
    const char* allowed[] = {"NO", "ON", "O(", ")O", "(N", "N)", "((", "))"};

    for(uint8_t i = 0; i < (sizeof(allowed) / sizeof(char*)); i++)
    {
        if(c1 == allowed[i][0] && c2 == allowed[i][1])
            return 1;
    }

    return 0;
}


// 0 = NO ERRORS
// 1 = ERRORS
uint8_t syntax_check(element* elements)
{
    uint32_t i;

    i = 0;
    while(1)
    {
        if(elements[i+1].type == 'E')
            break;

        if(elements[i].type == '(' && elements[i+1].type == '-')
        {
            // Exception, do not check
        }
        else // Main check
        {
            char c1, c2;

            if(IS_OPERATOR(elements[i].type))                               c1 = 'O';
            if(elements[i].type == 'I' || elements[i].type == 'F')          c1 = 'N';
            if(IS_BRACE(elements[i].type))                                  c1 = elements[i].type;


            if(IS_OPERATOR(elements[i+1].type))                             c2 = 'O';
            if(elements[i+1].type == 'I' || elements[i+1].type == 'F')      c2 = 'N';
            if(IS_BRACE(elements[i+1].type))                                c2 = elements[i+1].type;

            if(!is_valid_syntax(c1, c2))
                return 1;
        }

        i++;
    }

    return 0;
}


uint32_t str_len(char* s)
{
    const char* start = s;

    while(*(s++) != '\0');

    return s-start-1;
}


// 1 = Error
// 0 = Success
uint8_t preprocess(char* input, const uint32_t bufsize, char* variable)
{
    uint32_t i, len;

    len = str_len(input);

    // Wrap
    if((len+1+2) > bufsize)
        return 1;
    input[len+2] = '\0';
    input[len+1] = ')';
    for(i = len; i != 0; i--)
        input[i] = input[i-1];
    input[0] = '(';
    len += 2;

    // Variable
    for(i = 0; i < len-1; i++) // Check if AA or AAA without operator in between
    {
        if(input[i] == VARIABLE_CHAR && input[i+1] == VARIABLE_CHAR)
            return 1;
    }
    for(i = 0; i < len; i++)   // Fill in variable
    {
        if(input[i] == VARIABLE_CHAR)
        {
            uint32_t variable_pos = i;
            uint32_t variable_len = str_len(variable);
            uint32_t new_len = len+variable_len;

            if(variable_len == 0) // Variable is used, but not defined
                return 1;

            if(new_len > bufsize) // Filling in variable is too big for input buffer
                return 1;

            // Set end
            input[new_len-1] = '\0';

            // Shift
            for(i = new_len-2; i >= (variable_pos + variable_len); i--)
                input[i] = input[i-(variable_len-1)];

            // Write varible
            for(i = 0; i < variable_len; i++)
                input[variable_pos+i] = variable[i];

            len += variable_len-1;
        }
    }

    return 0;
}


uint8_t calculate_output_from_input(char* input, char* output, char* variable, element* elements)
{
    if(str_len(input) == 0)
    {
        sprintf(output, "%s", "str_len(input)=0");
        return 2;
    }

    // Variable = "1234"
    if(preprocess(input, INPUT_BUFSIZE, variable))
    {
        sprintf(output, "%s", "preprocess()");
        return 1; // Error: Preprocessing exceeds buffer-size
    }

    // Write into buffer
    if(str_to_elements(input, elements, ELEMENTS_BUFSIZE-1))
    {
        sprintf(output, "%s", "str_to_elements()");
        return 1; // Error: Input exceeds element-buffer size
    }

    // Nothing can go wrong with parsing numbers
    parse_numbers(elements);

    // Check syntax rules
    if(syntax_check(elements))
    {
        sprintf(output, "%s", "syntax_check()");
        return 1;
    }


    // Calculate
    element res = calculate_elements(elements);
    if(res.type != 'I' && res.type != 'F')
    {
        sprintf(output, "%s", "calculate_elements()");
        return 1;
    }

    // Print output
    if(res.type == 'I')
        sprintf(output, "%ld", res.var.i);
        
    if(res.type == 'F')
    {
        dtostrf(res.var.f, -(OUTPUT_BUFSIZE-1), 9, output);     // -width, for left adjustment
        
        // Cut leading zeroes
        uint8_t i = str_len(output)-1;
        while(output[i] == '0' || output[i] == ' ')
        {
            if(output[i-1] == '.')  // Do not cut the last zero
                break;

            output[i--] = '\0';
        }
    }
        

    return 0;
}


#ifdef CONSOLE_AVAILABLE

void print_elements(element* elements)
{
    uint32_t i = 0;

    while(1)
    {
        // Gemeinsame Attribute
        printf("index = [%d]\n", i);
        printf("type  = %c\n", elements[i].type);

        // Zahlenspezifische Attribute
        if((elements[i].type == 'I') || (elements[i].type == 'F'))
        {
            printf("int   = %d\n", elements[i].i);
            printf("float = %.9f\n\n", elements[i].f);
        }

        if(elements[i].type == 'E')
            break;

        i++;
    }
}


void print_elements_as_string(element* elements)
{
    uint32_t i = 0;

    while(1)
    {
        if(elements[i].type == 'E')
            break;

        if(elements[i].type == 'Z')
        {
            i++;
            continue;
        }

        // Print
        if(elements[i].type == 'I')
            printf("%d", elements[i].i);
        else if(elements[i].type == 'F')
            printf("%.9f", elements[i].f);
        else
            printf("%c", elements[i].type);

        i++;
    }

    printf("\n");
}

#endif
