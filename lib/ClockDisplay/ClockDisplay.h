//
// Created by mexomagno on 23-10-18.
//

#ifndef DACLAK_CLOCKDISPLAY_H
#define DACLAK_CLOCKDISPLAY_H

#define N_DIGITS 6

#include <Arduino.h>
#include <Time.h>

class ClockDisplay{
    /**
     * Represents the clock display. Provides methods to display internal state
     */
public:
    ClockDisplay(unsigned char, unsigned char, unsigned char);
    void showDate();
    void showText(char []);
    void setTzOffset(double);
    void setTextDelay(unsigned int);
    void setDateDelay(unsigned int);
    void update();  // TODO: Call internally
private:
    unsigned char input_pin;
    unsigned char shift_pin;
    unsigned char latch_pin;
    char displayed_digits [N_DIGITS+1];  // Digits that should be displayed on next update
    bool is_showing_text = false;
    bool is_showing_date = false;
    double tz_offset;
    char text_to_show[256];  // Text to display
    unsigned long last_text_millis;
    unsigned long date_millis;
    unsigned int TEXT_SCROLL_DELAY = 300;
    unsigned int DATE_DELAY = 2000;
    void putDate();
    void putTime();
    void putText();
    bool checkTextRotation();
    static void charToSegments(char, unsigned char &, unsigned char &);
    void updateSegment(unsigned char, unsigned char);
};

#endif //DACLAK_CLOCKDISPLAY_H
