//
// Created by mexomagno on 23-10-18.
//

#include "ClockDisplay.h"
#define NOP __asm__ __volatile__ ("nop\n\t")  // delay 62.5ns on a 16MHz AtMega
unsigned char ClockDisplay::input_pin;
unsigned char ClockDisplay::shift_pin;
unsigned char ClockDisplay::latch_pin;
double ClockDisplay::tz_offset = 0.0;
bool ClockDisplay::is_showing_text = false;
bool ClockDisplay::is_showing_date = false;
unsigned long ClockDisplay::last_text_millis;
unsigned long ClockDisplay::date_millis;
unsigned int ClockDisplay::TEXT_SCROLL_DELAY = DEFAULT_TEXT_SCROLL_DELAY;
unsigned int ClockDisplay::DATE_DELAY = DEFAULT_DATE_DELAY;
char ClockDisplay::displayed_digits[N_DIGITS+1];
char ClockDisplay::text_to_show[256];
unsigned long ClockDisplay::last_refresh_micros = 0;


ClockDisplay::ClockDisplay(unsigned char input_p, unsigned char shift_p, unsigned char latch_p){
    input_pin = input_p;
    shift_pin = shift_p;
    latch_pin = latch_p;
    // Enable pins
    pinMode(input_pin, OUTPUT);
    pinMode(shift_pin, OUTPUT);
    pinMode(latch_pin, OUTPUT);
    strcpy(displayed_digits, "000000");
    strcpy(text_to_show, "");
}

void ClockDisplay::setTzOffset(double new_offset) {
    tz_offset = new_offset;
}

void ClockDisplay::setTextDelay(unsigned int new_delay) {
    TEXT_SCROLL_DELAY = new_delay;
}

void ClockDisplay::setDateDelay(unsigned int new_delay){
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
    for (unsigned char i = 0; i < N_DIGITS; i++){
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



/**
 * Draws current digits into the display
 */
void ClockDisplay::update() {
    // Check if text is being displayed yet
    if (ClockDisplay::checkTextRotation()){
        // Text still being displayed, stop updating
    } else if (ClockDisplay::is_showing_date){ // Check if is showing date
        unsigned long now = millis();
        if (date_millis == 0)
            date_millis = now;
        if (now - date_millis < DATE_DELAY){
            // Date still must be shown
            //date_millis = now - (now - date_millis - DATE_DELAY);
            putDate();
        } else {
            // Stop showing date
            is_showing_date = false;
        }
    } else {
        putTime();
    }
    // TODO: Draw with delay
    // Display current digits
//    for (unsigned int i=0; i < strlen(displayed_digits); i++){
//        char st [30]; sprintf(st, "%c ", displayed_digits[i]);
//        Serial.print(st);
//    }
//    Serial.println();
    unsigned char part1, part2;
    charToSegments(displayed_digits[strlen(displayed_digits) - 1], part1, part2);
    updateSegment(part1, part2);
}

void ClockDisplay::putTime() {
    time_t t_now = now() + (time_t)(3600*tz_offset);
    // Time to digits
    char str_time[7];sprintf(str_time, "%02d%02d%02d", hour(t_now), minute(t_now), second(t_now));
    // Set digits and draw
    strcpy(displayed_digits, str_time);
}

void ClockDisplay::putDate() {
    time_t t_now = now() + (time_t)(3600*tz_offset);
    // Time to digits
    char str_date[7];sprintf(str_date,"%02d%02d%02d", day(t_now), month(t_now), year(t_now)%1000);
    strcpy(displayed_digits, str_date);
}

void ClockDisplay::putText(){
    for (unsigned char i = 0; i < N_DIGITS; i++){
        if (text_to_show[i] != '\0')
            displayed_digits[i] = text_to_show[i];
        else {
            // Fill with spaces
            for (unsigned char j = i; j < N_DIGITS ; j++){
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
bool ClockDisplay::checkTextRotation() {
    if (!ClockDisplay::is_showing_text)
        return false;
    unsigned long now = millis();
    if (last_text_millis == 0) {
        last_text_millis = now;
    }
    // Check if text should be rotated
    if (now - last_text_millis > TEXT_SCROLL_DELAY){
        last_text_millis = now - (now - last_text_millis - TEXT_SCROLL_DELAY);
        // Rotate chars
        for (unsigned int i = 0; i < strlen(text_to_show); i++){
            text_to_show[i] = text_to_show [i+1];
            if (text_to_show[i] == '\0')
                break;
        }
        if (strcmp(text_to_show, "") == 0){
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
    switch(c){
        case '0':
            out1 = B11111100;
            out2 = B00000000;
            return;
        case '1':
            out1 = B01100000;
            out2 = B00000000;
            return;
        case '2':
            out1 = B11011011;
            out2 = B00000000;
            return;
        case '3':
            out1 = B11110011;
            out2 = B00000000;
            return;
        case '4':
            out1 = B01100111;
            out2 = B00000000;
            return;
        case '5':
            out1 = B10110111;
            out2 = B00000000;
            return;
        case '6':
            out1 = B10111111;
            out2 = B00000000;
            return;
        case '7':
            out1 = B11100000;
            out2 = B00000000;
            return;
        case '8':
            out1 = B11111111;
            out2 = B00000000;
            return;
        case '9':
            out1 = B11110111;
            out2 = B00000000;
            return;
        case 'A':
        case 'a':
            out1 = B11101111;
            out2 = B00000000;
            return;
        case 'B':
        case 'b':
            out1 = B11110001;
            out2 = B01001000;
            return;
        case 'C':
        case 'c':
            out1 = B10011100;
            out2 = B00000000;
            return;
        case 'D':
        case 'd':
            out1 = B11110001;
            out2 = B01001000;
            return;
        case 'E':
        case 'e':
            out1 = B10011111;
            out2 = B00000000;
            return;
        case 'F':
        case 'f':
            out1 = B10001111;
            out2 = B00000000;
            return;
        case 'G':
        case 'g':
            out1 = B10111101;
            out2 = B00000000;
            return;
        case 'H':
        case 'h':
            out1 = B01101111;
            out2 = B00000000;
            return;
        case 'I':
        case 'i':
            out1 = B10010000;
            out2 = B01001000;
            return;
        case 'J':
        case 'j':
            out1 = B01111000;
            out2 = B00000000;
            return;
        case 'K':
        case 'k':
            out1 = B00001110;
            out2 = B00100100;
            return;
        case 'L':
        case 'l':
            out1 = B00011100;
            out2 = B00000000;
            return;
        case 'M':
        case 'm':
            out1 = B01101100;
            out2 = B10100000;
            return;
        case 'N':
        case 'n':
            out1 = B01101100;
            out2 = B10100000;
            return;
        case 'O':
        case 'o':
            out1 = B11111100;
            out2 = B00000000;
            return;
        case 'P':
        case 'p':
            out1 = B11001111;
            out2 = B00000000;
            return;
        case 'Q':
        case 'q':
            out1 = B11111100;
            out2 = B00000100;
            return;
        case 'R':
        case 'r':
            out1 = B11001111;
            out2 = B00000100;
            return;
        case 'S':
        case 's':
            out1 = B10110111;
            out2 = B00000000;
            return;
        case 'T':
        case 't':
            out1 = B10000000;
            out2 = B01001000;
            return;
        case 'U':
        case 'u':
            out1 = B01111100;
            out2 = B00000000;
            return;
        case 'V':
        case 'v':
            out1 = B00001100;
            out2 = B00110000;
            return;
        case 'W':
        case 'w':
            out1 = B01101100;
            out2 = B00010100;
            return;
        case 'X':
        case 'x':
            out1 = B00000000;
            out2 = B10110100;
            return;
        case 'Y':
        case 'y':
            out1 = B00000000;
            out2 = B10101000;
            return;
        case 'Z':
        case 'z':
            out1 = B10010000;
            out2 = B00110000;
            return;
        case '+':
            out1 = B00000011;
            out2 = B01001000;
            return;
        case '-':
            out1 = B00000011;
            out2 = B00000000;
            return;
        case '/':
            out1 = B00000000;
            out2 = B00110000;
            return;
        case '\\':
            out1 = B00000000;
            out2 = B10000100;
            return;
        case '*':
            out1 = B00000011;
            out2 = B11111100;
            return;
        default:
            out1 = B00000011;
            out2 = B11111100;
            return;
    }
}
//
//void ClockDisplay::displayDigits(){
//
//}



void ClockDisplay::updateSegment(unsigned char part1, unsigned char part2) {
    // TODO: Update each segment separately
    for (char p = 1; p <= 2; p++){
        for (unsigned char i = 0; i < 8; i++){
            digitalWrite(input_pin, bitRead(p == 1 ? part1 : part2, 7-i));
            NOP;
            digitalWrite(shift_pin, HIGH);
            NOP;
            digitalWrite(shift_pin, LOW);
            NOP;
        }
    }
    digitalWrite(latch_pin, HIGH);
    NOP;
    digitalWrite(latch_pin, LOW);
    NOP;
}

/**
 * Starts display refresh routine
 */
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
}



/**
TODO:
 * Change "update()" meaning to mean "Show next digit in display" instead of "dump all digits in each display"
 * */