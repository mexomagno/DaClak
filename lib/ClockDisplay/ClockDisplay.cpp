//
// Created by mexomagno on 23-10-18.
//

#include "ClockDisplay.h"
#define NOP __asm__ __volatile__("nop\n\t")  // delay 62.5ns on a 16MHz AtMega
unsigned char ClockDisplay::input_pin;
unsigned char ClockDisplay::shift_pin;
unsigned char ClockDisplay::latch_pin;
unsigned char ClockDisplay::DIGIT_PINS[];
double ClockDisplay::tz_offset = 0.0;
bool ClockDisplay::is_showing_text = false;
bool ClockDisplay::is_showing_date = false;
unsigned long ClockDisplay::last_text_millis;
unsigned long ClockDisplay::date_millis;
unsigned long ClockDisplay::last_dots_change = millis();
unsigned char ClockDisplay::last_second = second();
unsigned int ClockDisplay::TEXT_SCROLL_DELAY = DEFAULT_TEXT_SCROLL_DELAY;
unsigned int ClockDisplay::DATE_DELAY = DEFAULT_DATE_DELAY;
char ClockDisplay::displayed_digits[N_DIGITS + 1];
char ClockDisplay::text_to_show[256];
//unsigned long ClockDisplay::last_refresh_micros = 0;

// TODO: PROVIDE TIDIER WAY OF SPECIFYING SELECTOR PORTS. FOR NOW, ARDUINO'S PORTB IS HARDCODED
void digitalWriteFast(unsigned char pin, bool val) {
    uint8_t *port = &PORTD;
    if (pin >= 8)
        port = &PORTB;
    if (val)
        bitSet(*port, pin % 8);
    else
        bitClear(*port, pin % 8);
}

/**
 * Reorder segments according to real pcb connections
 * 
 * Input order is
 * char1: ABCDEFGH
 * char2: IJKLMNXX
 * 
 * Actual PCB order is: 
 * char1: AFBKJIG-
 * char2: HLMNCEDX
 * 
 * We do not worry about X as it is set by the seconds blinker routine
 */
void reorderSegments(unsigned char &char1, unsigned char &char2) {
    unsigned char newchar1 = 0;
    unsigned char newchar2 = 0;
    unsigned int all = char2 | char1 << 8;

    unsigned char b1_indexes[] = {0 /*A*/, 5 /*F*/, 1 /*B*/, 8 + 2 /*K*/, 8 + 1 /*J*/, 8 + 0 /*I*/, 6 /*G*/};
    unsigned char b2_indexes[] = {7 /*H*/, 8 + 3 /*L*/, 8 + 4 /*M*/, 8 + 5 /*N*/, 2 /*C*/, 4 /*E*/, 3 /*D*/};
    for (int digit_index = 0; digit_index < sizeof(b1_indexes) / sizeof(unsigned char); digit_index++) {
        // first byte
        newchar1 |= bitRead(all, 15 - b1_indexes[digit_index]) << (7 - digit_index);
        // second byte
        newchar2 |= bitRead(all, 15 - b2_indexes[digit_index]) << (7 - digit_index);
    }
    char1 = newchar1;
    char2 = newchar2;
}

ClockDisplay::ClockDisplay(unsigned char input_p, unsigned char shift_p, unsigned char latch_p, unsigned char digit_pins[]) {
    input_pin = input_p;
    shift_pin = shift_p;
    latch_pin = latch_p;
    for (unsigned char i = 0; i < 6; i++)
        DIGIT_PINS[i] = digit_pins[i];
    // Enable pins
    pinMode(input_pin, OUTPUT);
    pinMode(shift_pin, OUTPUT);
    pinMode(latch_pin, OUTPUT);
    strcpy(displayed_digits, "000000");
    strcpy(text_to_show, "");

    // TODO: unhardcode the following mess:
    // Setup PORTB output
    DDRB = DDRB | B00111111;
    PORTB = 0;
}

void ClockDisplay::setTzOffset(double new_offset) {
    tz_offset = new_offset;
}

void ClockDisplay::setTextDelay(unsigned int new_delay) {
    TEXT_SCROLL_DELAY = new_delay;
}

void ClockDisplay::setDateDelay(unsigned int new_delay) {
    DATE_DELAY = new_delay;
}

void ClockDisplay::showDate() {
    putDate();
    is_showing_date = true;
    date_millis = 0;
    is_showing_text = false;
}

/**
 * Displays some text as a sliding animation
 *
 * TODO: Don't block!
 * @param text
 */
void ClockDisplay::showText(char text[]) {
    // Add pading spaces
    char spaces[N_DIGITS + 1];
    for (unsigned char i = 0; i < N_DIGITS; i++) {
        spaces[i] = ' ';
    }
    spaces[N_DIGITS] = '\0';
    char new_text[N_DIGITS + strlen(text)];
    sprintf(new_text, "%s%s", spaces, text);
    // Put text
    strcpy(text_to_show, new_text);
    is_showing_text = true;
    last_text_millis = 0;
    is_showing_date = false;
}

void ClockDisplay::putTime() {
    time_t t_now = now() + (time_t)(3600 * tz_offset);
    // Time to digits
    char str_time[7];
    sprintf(str_time, "%02d%02d%02d", hour(t_now), minute(t_now), second(t_now));
    // Set digits
    strcpy(displayed_digits, str_time);
}

void ClockDisplay::putDate() {
    time_t t_now = now() + (time_t)(3600 * tz_offset);
    // Time to digits
    char str_date[7];
    sprintf(str_date, "%02d%02d%02d", day(t_now), month(t_now), year(t_now) % 1000);
    strcpy(displayed_digits, str_date);
}

void ClockDisplay::putText() {
    for (unsigned char i = 0; i < N_DIGITS; i++) {
        if (text_to_show[i] != '\0')
            displayed_digits[i] = text_to_show[i];
        else {
            // Fill with spaces
            for (unsigned char j = i; j < N_DIGITS; j++) {
                displayed_digits[j] = ' ';
            }
            break;
        }
    }
}

/**
 * Checks if text must be rotated, and does it.
 * @return true if text is still displayed, false otherwise
 */
bool ClockDisplay::checkTextRotation(unsigned long now) {
    if (!ClockDisplay::is_showing_text)
        return false;
    if (last_text_millis == 0) {
        last_text_millis = now;
    }
    // Check if text should be rotated
    if (now - last_text_millis > TEXT_SCROLL_DELAY) {
        last_text_millis = now - (now - last_text_millis - TEXT_SCROLL_DELAY);
        // Rotate chars
        for (unsigned int i = 0; i < strlen(text_to_show); i++) {
            text_to_show[i] = text_to_show[i + 1];
            if (text_to_show[i] == '\0')
                break;
        }
        if (strcmp(text_to_show, "") == 0) {
            // Finished text scroll
            is_showing_text = false;
        }
    }
    putText();
    return is_showing_text;
}

/**
 * Represents a char c as an array of segments.
 *
 * Segments are as follows:
 *
 *    ___________
 *  _|_____A_____|_
 * | |\\  | |  //| |
 * |F| \I\|J|/K/ |B|
 * |_|__\\|_|//__|_|
 *  _|_G__|_|__H_|_
 * | |  //| |\\  | |
 * |E| /L/|M|\N\ |C|
 * |_|//__|_|__\\|_|
 *   |_____D_____|
 *
 *
 * Return values represent:
 *
 * out1 = ABCDEFGH
 * out2 = IJKLMNXX  (X's are ignored)
 *
 * @param c
 * @param out1
 * @param out2
 * @return
 */
void ClockDisplay::charToSegments(char c, unsigned char &out1, unsigned char &out2) {
    switch (c) {
        case '0':
            out1 = B11111100;
            out2 = B00000000;
            // out1 = B11111111;
            // out2 = B11111100;
            break;
        case '1':
            out1 = B01100000;
            out2 = B00000000;
            break;
        case '2':
            out1 = B11011011;
            out2 = B00000000;
            break;
        case '3':
            out1 = B11110011;
            out2 = B00000000;
            break;
        case '4':
            out1 = B01100111;
            out2 = B00000000;
            break;
        case '5':
            out1 = B10110111;
            out2 = B00000000;
            break;
        case '6':
            out1 = B10111111;
            out2 = B00000000;
            break;
        case '7':
            out1 = B11100000;
            out2 = B00000000;
            break;
        case '8':
            out1 = B11111111;
            out2 = B00000000;
            break;
        case '9':
            out1 = B11110111;
            out2 = B00000000;
            break;
        case 'A':
        case 'a':
            out1 = B11101111;
            out2 = B00000000;
            break;
        case 'B':
        case 'b':
            out1 = B11110001;
            out2 = B01001000;
            break;
        case 'C':
        case 'c':
            out1 = B10011100;
            out2 = B00000000;
            break;
        case 'D':
        case 'd':
            out1 = B11110001;
            out2 = B01001000;
            break;
        case 'E':
        case 'e':
            out1 = B10011111;
            out2 = B00000000;
            break;
        case 'F':
        case 'f':
            out1 = B10001111;
            out2 = B00000000;
            break;
        case 'G':
        case 'g':
            out1 = B10111101;
            out2 = B00000000;
            break;
        case 'H':
        case 'h':
            out1 = B01101111;
            out2 = B00000000;
            break;
        case 'I':
        case 'i':
            out1 = B10010000;
            out2 = B01001000;
            break;
        case 'J':
        case 'j':
            out1 = B01111000;
            out2 = B00000000;
            break;
        case 'K':
        case 'k':
            out1 = B00001110;
            out2 = B00100100;
            break;
        case 'L':
        case 'l':
            out1 = B00011100;
            out2 = B00000000;
            break;
        case 'M':
        case 'm':
            out1 = B01101100;
            out2 = B10100000;
            break;
        case 'N':
        case 'n':
            out1 = B01101100;
            out2 = B10000100;
            break;
        case 'O':
        case 'o':
            out1 = B11111100;
            out2 = B00000000;
            break;
        case 'P':
        case 'p':
            out1 = B11001111;
            out2 = B00000000;
            break;
        case 'Q':
        case 'q':
            out1 = B11111100;
            out2 = B00000100;
            break;
        case 'R':
        case 'r':
            out1 = B11001111;
            out2 = B00000100;
            break;
        case 'S':
        case 's':
            out1 = B10110111;
            out2 = B00000000;
            break;
        case 'T':
        case 't':
            out1 = B10000000;
            out2 = B01001000;
            break;
        case 'U':
        case 'u':
            out1 = B01111100;
            out2 = B00000000;
            break;
        case 'V':
        case 'v':
            out1 = B00001100;
            out2 = B00110000;
            break;
        case 'W':
        case 'w':
            out1 = B01101100;
            out2 = B00010100;
            break;
        case 'X':
        case 'x':
            out1 = B00000000;
            out2 = B10110100;
            break;
        case 'Y':
        case 'y':
            out1 = B00000000;
            out2 = B10101000;
            break;
        case 'Z':
        case 'z':
            out1 = B10010000;
            out2 = B00110000;
            break;
        case '+':
            out1 = B00000011;
            out2 = B01001000;
            break;
        case '-':
            out1 = B00000011;
            out2 = B00000000;
            break;
        case '/':
            out1 = B00000000;
            out2 = B00110000;
            break;
        case '\\':
            out1 = B00000000;
            out2 = B10000100;
            break;
        case '*':
            out1 = B00000011;
            out2 = B11111100;
            break;
        case ' ':
            out1 = B00000000;
            out2 = B00000000;
            break;
        case ',':
        case '.':
            out1 = B00000000;
            out2 = B00010000;
            break;
        case 1:
            out1 = B10000000;
            out2 = B00000000;
            break;
        case 2:
            out1 = B01000000;
            out2 = B00000000;
            break;
        case 3:
            out1 = B00100000;
            out2 = B00000000;
            break;
        case 4:
            out1 = B00010000;
            out2 = B00000000;
            break;
        case 5:
            out1 = B00001000;
            out2 = B00000000;
            break;
        case 6:
            out1 = B00000100;
            out2 = B00000000;
            break;
        default:
            out1 = B00000011;
            out2 = B11111100;
            break;
    }
    reorderSegments(out1, out2);
}

/**
 * Using the shift registers, turns segments on an off according to part1 and part2
 * @param part1: First 8 segment parts (A to H)
 * @param part2: Last 6 segments + dots (I to N, dots)
 */
void ClockDisplay::updateSegment(unsigned char part1, unsigned char part2) {
    // TODO: Update each segment separately
    for (char p = 1; p <= 2; p++) {
        for (unsigned char i = 0; i < 8; i++) {
            // Put data in serial input
            digitalWriteFast(input_pin, bitRead(p == 1 ? part1 : part2, 7 - i));
            NOP;
            // Shift to right
            digitalWriteFast(shift_pin, HIGH);
            NOP;
            digitalWriteFast(shift_pin, LOW);
            NOP;
        }
    }
    // Set values on shift register outputs
    digitalWriteFast(latch_pin, HIGH);
    NOP;
    digitalWriteFast(latch_pin, LOW);
    NOP;
}

/**
 * Starts display refresh routine
 *
 * USING THIS IS DISCOURAGED AS IT MESSES UP TIME ACCURACY
 */
/*
void ClockDisplay::begin(){
    // Disable counter overflow interrupts
    TIMSK2 &= ~(1 << TOIE2);
    // Enable Compare Match A interrupts
    TIMSK2 |= (1 << OCIE2A);
    // Clear TCCR2A and TCCR2B registers
    TCCR2A = 0;
    TCCR2B = 0;
    // Set CTC mode
    TCCR2A |= (1 << WGM21);
    // Set prescaler to 8
    TCCR2B |= (1 << CS21);
    // Set value to match for interrupt triggering
    // Timer 2 counter overflows at 255
    OCR2A = 255;
}

// Update interrupts
ISR(TIMER2_COMPA_vect){
    unsigned long us = micros();
    if (ClockDisplay::last_refresh_micros == 0){
        ClockDisplay::last_refresh_micros = us;
    }
    if (us - ClockDisplay::last_refresh_micros > ClockDisplay::REFRESH_DELAY_MICROS){
        ClockDisplay::update();  // TODO: Consider doing everything here instead of calling function. Context change could cost few microseconds
        ClockDisplay::last_refresh_micros = us - (us - ClockDisplay::last_refresh_micros - ClockDisplay::REFRESH_DELAY_MICROS);
    }
}*/

char index = 0;

/**
 * Draws current digits into the display
 */
void ClockDisplay::update() {
    unsigned long current_millis = micros() / 1000;
    // Check if text is being displayed yet
    if (ClockDisplay::checkTextRotation(current_millis)) {
        // Text still being displayed, stop updating
    } else if (ClockDisplay::is_showing_date) {  // Check if is showing date
        if (date_millis == 0)
            date_millis = current_millis;
        if (current_millis - date_millis < DATE_DELAY) {
            // Date still must be shown
            //date_millis = current_millis - (current_millis - date_millis - DATE_DELAY);
            putDate();
        } else {
            // Stop showing date
            is_showing_date = false;
        }
    } else {
        putTime();
    }
    // TODO: Draw with delay
    unsigned char part1, part2;
    charToSegments(displayed_digits[index], part1, part2);
    // Check dots status
    if (second() != last_second) {
        last_second = second();
        last_dots_change = current_millis;
    }
    // show dots
    if (current_millis - last_dots_change < DOTS_DELAY && !is_showing_text && !is_showing_date)
        bitSet(part2, 0);
    else
        bitClear(part2, 0);
    // Turn off previous digit
    digitalWriteFast(DIGIT_PINS[(index + N_DIGITS - 1) % N_DIGITS], 0);
    // bitClear(PORTB, (index + N_DIGITS - 1) % N_DIGITS);

    // Update new values
    updateSegment(part1, part2);

    // Turn on current digit
    digitalWriteFast(DIGIT_PINS[index], 1);
    // bitSet(PORTB, index);

    // Prepare for next iteration
    index = (index + 1) % N_DIGITS;
}