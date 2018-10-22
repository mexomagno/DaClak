//
// Created by mexomagno on 21-10-18.
//

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <string.h>
#include <Time.h>

// SoftwareSerial *bt_module = new SoftwareSerial(2, 3);

void setup(){
    //pinMode(13, OUTPUT);
    Serial.begin(9600);
    Serial.println("Started serial");
}

void printTime(){
    Serial.print("Time: ");
    Serial.print(hour());
    Serial.print(":");
    Serial.print(minute());
    Serial.print(":");
    Serial.println(second());
}

void loop(){
    delay(1000);
    printTime();
}