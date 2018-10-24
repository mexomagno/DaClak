//
// Created by mexomagno on 21-10-18.
//

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <ClockDisplay.h>
#include <BTConnection.h>

double tz_offset = 0;
unsigned long baud_rate = 9600;
ClockDisplay display(4, 5, 6);
BTConnection bt_connection(2, 3, baud_rate);

void parseCommand();

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

/**
TODO: Uncouple stuff:
 - Decide if clock functionality will be take by ClockDisplay
 - Parse BTConnection commands in the sketch, to do stuff with clock and ClockDisplay

*/