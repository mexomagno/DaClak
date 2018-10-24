//
// Created by mexomagno on 23-10-18.
//

#include "ClockDisplay.h"

ClockDisplay::ClockDisplay(unsigned char input_pin, unsigned char shift_pin, unsigned char latch_pin) {
    this->input_pin = input_pin;
    this->shift_pin = shift_pin;
    this->latch_pin = latch_pin;
    // Enable pins
    pinMode(input_pin, OUTPUT);
    pinMode(shift_pin, OUTPUT);
    pinMode(latch_pin, OUTPUT);
    strcpy(displayed_digits, "000000");
    strcpy(text_to_show, "");
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
    for (unsigned int i=0; i < strlen(displayed_digits); i++){
        char st [30]; sprintf(st, "%c ", displayed_digits[i]);
        Serial.print(st);
    }
    Serial.println();
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
