//
// Created by mexomagno on 21-10-18.
//

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Time.h>
#include <TimeAlarms.h>

#define N_DIGITS 6
double tz_offset = 0;

//void print(char *, args**){
//    if (!Serial.availableForWrite())
//        return;
//
//}

class ClockDisplay{
    /**
     * Represents the clock display. Provides methods to display internal state
     */
public:
    ClockDisplay(unsigned char, unsigned char, unsigned char);
    void showTime();
private:
    unsigned char input_pin;
    unsigned char shift_pin;
    unsigned char latch_pin;
    char displayed_digits [N_DIGITS+1];
    void setDigits(char []);
};
ClockDisplay::ClockDisplay(unsigned char input_pin, unsigned char shift_pin, unsigned char latch_pin) {
    this->input_pin = input_pin;
    this->shift_pin = shift_pin;
    this->latch_pin = latch_pin;
    // Enable pins
    pinMode(input_pin, OUTPUT);
    pinMode(shift_pin, OUTPUT);
    pinMode(latch_pin, OUTPUT);
    setDigits((char*)("000000"));
}

void ClockDisplay::showTime() {
    time_t t_now = now() + (time_t)(3600*tz_offset);
    // Time to digits
    char str_time[7];sprintf(str_time, "%02d%02d%02d", hour(t_now), minute(t_now), second(t_now));
    // Set digits and draw
    setDigits(str_time);
    char st[30];sprintf(st, "Display: %s", str_time);
    Serial.println(st);
}

/**
 * Sets and draws the digits in the display
 * @param s : Array of characters to show
 */
void ClockDisplay::setDigits(char * s) {
    // Update digits
    strcpy(displayed_digits, s);
    // Draw digits
    for (unsigned int i=0; i < strlen(displayed_digits); i++){
        char st [30]; sprintf(st, "Digit %d: %c", i, displayed_digits[i]);
        Serial.println(st);
    }
}

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
BTConnection *bt_connection = new BTConnection(2, 3, baud_rate);
ClockDisplay *display = new ClockDisplay(4, 5, 6);

void setup(){
    // Start serial for debugging
    Serial.begin(baud_rate);
    Serial.println("Started serial");
    // Start bt module
    bt_connection->begin();
    Serial.print("Started BT module. Baud Rate: ");
    Serial.println(baud_rate);
}

void loop(){
//    bt_connection->listen();
    display->showTime();
    delay(500);
}
