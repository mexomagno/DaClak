//
// Created by mexomagno on 21-10-18.
//

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Time.h>
#include <TimeAlarms.h>

#define N_DIGITS 6

double tz_offset = 0;

class ClockDisplay{
    /**
     * Represents the clock display. Provides methods to display internal state
     */
public:
    ClockDisplay(unsigned char, unsigned char, unsigned char);
    void showDate();
    void showText(char []);
    void update();  // TODO: Call internally
private:
    unsigned char input_pin;
    unsigned char shift_pin;
    unsigned char latch_pin;
    char displayed_digits [N_DIGITS+1];  // Digits that should be displayed on next update
    bool is_showing_text = false;
    bool is_showing_date = false;
    char text_to_show[256];  // Text to display
    unsigned long last_text_millis;
    unsigned long date_millis;
    const unsigned int TEXT_SCROLL_DELAY = 300;
    const unsigned int DATE_DELAY = 2000;
    void putDate();
    void putTime();
    void putText();
    bool checkTextRotation();

};


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
    // Store text
    strcpy(text_to_show, text);
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

ClockDisplay display(4, 5, 6);

class BTConnection{
    /**
     * Designed to abstract communication between BT module and the arduino
     */
public:
    BTConnection(unsigned char rx_pin, unsigned char tx_pin, unsigned long baud_rate);
    void begin();
    boolean send(char COMMAND[]);
    boolean parseCommand(char command []);
    void listen();
private:
    unsigned char rx_pin;
    unsigned char tx_pin;
    unsigned long baud_rate;
    SoftwareSerial *bt_module;
    char buffer[128];
};

BTConnection::BTConnection(unsigned char rx_pin, unsigned char tx_pin, unsigned long baud_rate) {
    this->rx_pin = rx_pin;
    this->tx_pin = tx_pin;
    this->baud_rate = baud_rate;
    // Initialize
    this->bt_module = new SoftwareSerial(this->rx_pin, this->tx_pin);
}

void BTConnection::begin() {
    this->bt_module->begin(this->baud_rate);
}

boolean BTConnection::parseCommand(char *command) {
    // get pretty time
    if (strcmp(command, "TIME") == 0) {
        time_t t = now();
        t += (time_t)(3600*tz_offset);
        char str_offset[15];
        dtostrf(tz_offset, 1, 1, str_offset);
        char zo[15];
        if (tz_offset != 0)
            sprintf(zo, " (GMT %s%s)", tz_offset > 0 ? "+" : "", str_offset);
        char s [128];
        sprintf(s, "Time: %02d:%02d:%02d%s\n", hour(t), minute(t), second(t), tz_offset == 0 ? "" : zo);
        this->send(s);
        return true;
    }
    // Get pretty date
    if (strcmp(command, "DATE") == 0) {
        time_t t = now();
        t += (time_t)(3600*tz_offset);
        char str_offset[15];
        dtostrf(tz_offset, 1, 1, str_offset);
        char zo[15];
        if (tz_offset != 0)
            sprintf(zo, " (GMT %s%s)", tz_offset > 0 ? "+" : "", str_offset);
        char s [128];
        sprintf(s, "Date: %d/%d/%d%s\n", day(t), month(t), year(t), tz_offset == 0 ? "" : zo);
        this->send(s);
        display.showDate();
        return true;
    }
    // Get timestamp
    if (strcmp(command, "GT") == 0){
        char s [128];
        sprintf(s, "%ld\n", (long int)now());
        this->send(s);
        return true;
    }
    // Get timezone offset
    if (strcmp(command, "GZ") == 0){
        char str_offset[15];
        dtostrf(tz_offset, 1, 1, str_offset);
        char s [128];
        sprintf(s, "%s\n", str_offset);
        this->send(s);
        return true;
    }
    // Set timestamp
    if (command[0] == 'S' && command[1] == 'T'){
        command[0] = ' ';
        command[1] = ' ';
        char *endptr;
        long int timestamp = strtol(command, &endptr, 10);
        if (endptr - &command[0] != 12 || timestamp < 0)
            return false;
        setTime((time_t)timestamp);
        return true;
    }
    // Set timezone offset
    if (command[0] == 'S' && command[1] == 'Z'){
        command[0] = ' ';
        command[1] = ' ';
        char *endptr;
        tz_offset = strtod(command, &endptr);
        Serial.print("Timezone offset: ");
        Serial.println(tz_offset);
        return true;
    }
    else {
        // Display command as text
        display.showText(command);
    }
    return false;
}

void BTConnection::listen() {
    int index = 0;
    if (this->bt_module->available()){
        this->buffer[index++] = (char)this->bt_module->read();
        delayMicroseconds(100);
        while (this->bt_module->available()) {
            this->buffer[index++] = (char)this->bt_module->read();
            delayMicroseconds(100);
        }
        this->buffer[index] = '\0';
        Serial.print("Command: ");
        Serial.println(buffer);
        bool result = this->parseCommand(buffer);
        if (result){
            this->send("OK\n");
        } else {
            this->send("ERROR\n");
        }
    }
}

boolean BTConnection::send(char *COMMAND) {
    this->bt_module->print(COMMAND);
}

unsigned long baud_rate = 9600;
BTConnection bt_connection(2, 3, baud_rate);

void setup(){
    // Start serial for debugging
    Serial.begin(baud_rate);
    Serial.println("Started serial");
    // Start bt module
    bt_connection.begin();
    Serial.print("Started BT module. Baud Rate: ");
    Serial.println(baud_rate);
}

void loop(){
    bt_connection.listen();
    display.update();  // TODO: Update in non invasive timer interrupt
    delay(100);
}
