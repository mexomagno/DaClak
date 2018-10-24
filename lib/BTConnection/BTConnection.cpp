//
// Created by mexomagno on 23-10-18.
//

#include "BTConnection.h"


BTConnection::BTConnection(unsigned char rx_pin, unsigned char tx_pin, unsigned long baud_rate, ClockDisplay *display) {
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
        char s [20];
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
        char s [20];
        sprintf(s, "Date: %d/%d/%d%s\n", day(t), month(t), year(t), tz_offset == 0 ? "" : zo);
        this->send(s);
        display.showDate();
        return true;
    }
    // Get timestamp
    if (strcmp(command, "GT") == 0){
        char s [20];
        sprintf(s, "%ld\n", (long int)now());
        this->send(s);
        return true;
    }
    // Get timezone offset
    if (strcmp(command, "GZ") == 0){
        char str_offset[15];
        dtostrf(tz_offset, 1, 1, str_offset);
        char s [8];
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