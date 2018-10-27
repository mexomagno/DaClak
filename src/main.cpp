//
// Created by mexomagno on 21-10-18.
//

#include <Arduino.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <ClockDisplay.h>
#include <BTConnection.h>


void segmentsTest(unsigned char input_pin, unsigned char shift_pin, unsigned char latch_pin, int delay_ms){
    pinMode(input_pin, OUTPUT);
    pinMode(shift_pin, OUTPUT);
    pinMode(latch_pin, OUTPUT);
    int c = 0;
    while (true){
        for (char i = 0; i < 16; i++){
            digitalWrite(input_pin, i == c);
            delay(1);
            digitalWrite(shift_pin, HIGH);
            delay(1);
            digitalWrite(shift_pin, LOW);
            delay(1);
        }
        c = (c+1)%14;
        digitalWrite(latch_pin, HIGH);
        delay(1);
        digitalWrite(latch_pin, LOW);
        delay(1);
        delay(delay_ms);
    }
}


double tz_offset = 0;
unsigned long baud_rate = 9600;
ClockDisplay display(4, 5, 6);
BTConnection bt_connection(2, 3, baud_rate);


bool parseCommand(char *command){
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
        bt_connection.send(s);
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
        bt_connection.send(s);
        display.showDate();
        return true;
    }
    // Get timestamp
    if (strcmp(command, "GT") == 0){
        char s [20];
        sprintf(s, "%ld\n", (long int)now());
        bt_connection.send(s);
        return true;
    }
    // Get timezone offset
    if (strcmp(command, "GZ") == 0){
        char str_offset[15];
        dtostrf(tz_offset, 1, 1, str_offset);
        char s [8];
        sprintf(s, "%s\n", str_offset);
        bt_connection.send(s);
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
        display.setTzOffset(tz_offset);
        Serial.print("Timezone offset: ");
        Serial.println(tz_offset);
        return true;
    }
    // Show text
    if (command[0] == 'T' && command[1] == 'X'){
        char txt[256];
        strncpy(txt, command + 2, strlen(command)-1);
        display.showText(txt);
    }
    // Set text speed
    if (command[0] == 'S' && command[1] == 'X'){
        command[0] = ' ';
        command[1] = ' ';
        char *endptr;
        auto text_delay = (unsigned int)strtol(command, &endptr, 10);
        display.setTextDelay(text_delay);
        Serial.print("Text delay: ");
        Serial.println(text_delay);
        return true;
    }
    // Set date delay
    if (command[0] == 'S' && command[1] == 'D'){
        command[0] = ' ';
        command[1] = ' ';
        char *endptr;
        auto date_delay = (unsigned int)strtol(command, &endptr, 10);
        display.setDateDelay(date_delay);
        Serial.print("Date delay: ");
        Serial.println(date_delay);
        return true;
    }
    return false;
}


void setup(){
    // Start serial for debugging
    Serial.begin(baud_rate);
    Serial.println("Started serial");
    // Start bt module
    bt_connection.begin();
    Serial.print("Started BT module. Baud Rate: ");
    Serial.println(baud_rate);
//    display.begin();
}

void loop(){
    bt_connection.listen(&parseCommand);
    ClockDisplay::update();  // TODO: Update in non invasive timer interrupt
    delay(100);
}
