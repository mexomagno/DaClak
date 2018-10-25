//
// Created by mexomagno on 23-10-18.
//

#include "ClockDisplay.h"

ClockDisplay::ClockDisplay(unsigned char input_pin, unsigned char shift_pin, unsigned char latch_pin) {
    this->input_pin = input_pin;
    this->shift_pin = shift_pin;
    this->latch_pin = latch_pin;
    this->tz_offset = 0.0;
    // Enable pins
    pinMode(input_pin, OUTPUT);
    pinMode(shift_pin, OUTPUT);
    pinMode(latch_pin, OUTPUT);
    strcpy(displayed_digits, "000000");
    strcpy(text_to_show, "");
}

void ClockDisplay::setTzOffset(double new_offset) {
    this->tz_offset = new_offset;
}

void ClockDisplay::setTextDelay(unsigned int new_delay) {
    TEXT_SCROLL_DELAY = new_delay;
}

void ClockDisplay::setDateDelay(unsigned int new_delay){
    DATE_DELAY = new_delay;
}

void ClockDisplay::putTime() {
    time_t t_now = now() + (time_t)(3600*tz_offset);
    // Time to digits
    char str_time[7];sprintf(str_time, "%02d%02d%02d", hour(t_now), minute(t_now), second(t_now));
    // Set digits and draw
    strcpy(displayed_digits, str_time);
}

/**
 * Puts the current characters of the text to show
 */
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
    if (checkTextRotation()){
        // Text still being displayed, stop updating
    } else if (is_showing_date){ // Check if is showing date
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
    for (unsigned int i=0; i < strlen(displayed_digits); i++){
        char st [30]; sprintf(st, "%c ", displayed_digits[i]);
        Serial.print(st);
    }
    Serial.println();
    // TEST: Put last digit in segments
    unsigned char out1, out2;
    charToSegments(displayed_digits[N_DIGITS - 1], out1, out2);
    updateSegment(out1, out2);
}

/**
 * Checks if text must be rotated, and does it.
 * @return true if text is still displayed, false otherwise
 */
bool ClockDisplay::checkTextRotation() {
    if (!is_showing_text)
        return false;
    unsigned long now = millis();
    if (last_text_millis == 0) {
        last_text_millis = now;
    }
    // Check if text should be rotated
    if (now - last_text_millis > TEXT_SCROLL_DELAY){
        last_text_millis = now - (now - last_text_millis - TEXT_SCROLL_DELAY);
        // Rotate chars
        for (int i = 0; i < strlen(text_to_show); i++){
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

void ClockDisplay::putDate() {
    time_t t_now = now() + (time_t)(3600*tz_offset);
    // Time to digits
    char str_date[7];sprintf(str_date,"%02d%02d%02d", day(t_now), month(t_now), year(t_now)%1000);
    strcpy(displayed_digits, str_date);
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
static void ClockDisplay::charToSegments(char c, unsigned char &out1, unsigned char &out2) {
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
        default:
            out1 = B00000011;
            out2 = B11111100;
            return;
    }
}

void ClockDisplay::updateSegment(unsigned char part1, unsigned char part2) {
    for (unsigned char i = 0; i < 8; i++){
        digitalWrite(input_pin, bitRead(part1, i));
        digitalWrite(shift_pin, HIGH);
        delay(1);
        digitalWrite(shift_pin, LOW);
        delay(1);
    }
    digitalWrite(latch_pin, HIGH);
    delay(1);
    digitalWrite(latch_pin, LOW);
    delay(1);
}