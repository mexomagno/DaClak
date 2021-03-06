//
// Created by mexomagno on 23-10-18.
//

#ifndef DACLAK_CLOCKDISPLAY_H
#define DACLAK_CLOCKDISPLAY_H

#define N_DIGITS 6

#include <Arduino.h>
#include <Time.h>

class ClockDisplay {
    /**
     * Represents the clock display. Provides methods to display internal state
     */
   public:
    ClockDisplay(unsigned char, unsigned char, unsigned char, unsigned char[6]);
    void begin();
    void showDate();
    void showText(char[]);
    void setTzOffset(double);
    void setTextDelay(unsigned int);
    void setDateDelay(unsigned int);
    //    static const unsigned long REFRESH_DELAY_MICROS = 100000;
    void update();
    //    static unsigned long last_refresh_micros;
   private:
    static unsigned char input_pin;
    static unsigned char shift_pin;
    static unsigned char latch_pin;
    static unsigned char DIGIT_PINS[6];
    static char displayed_digits[N_DIGITS + 1];  // Digits that should be displayed on next update
    static bool is_showing_text;
    static bool is_showing_date;
    static double tz_offset;
    static char text_to_show[256];  // Text to display
    static unsigned long last_text_millis;
    static unsigned long date_millis;
    static unsigned long last_dots_change;
    static unsigned char last_second;
    static const unsigned int DOTS_DELAY = 500;
    static const unsigned int DEFAULT_TEXT_SCROLL_DELAY = 300;
    static unsigned int TEXT_SCROLL_DELAY;
    static const unsigned int DEFAULT_DATE_DELAY = 2000;
    static unsigned int DATE_DELAY;
    static void putDate();
    static void putTime();
    static void putText();
    static bool checkTextRotation(unsigned long);
    static void charToSegments(char, unsigned char &, unsigned char &);
    static void updateSegment(unsigned char, unsigned char);
};

#endif  //DACLAK_CLOCKDISPLAY_H
