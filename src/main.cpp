//
// Created by mexomagno on 21-10-18.
//

#import <Arduino.h>
#import <SoftwareSerial.h>
#import <string.h>

SoftwareSerial *bt_module = new SoftwareSerial(2, 3);


void setup(){
    pinMode(13, OUTPUT);
    Serial.begin(9600);
    Serial.println("Started serial");
    bt_module->begin(9600);
}

void loop(){
    while (bt_module->available()){
        Serial.write(bt_module->read());
    }
    while (Serial.available()){
        bt_module->write(Serial.read());
    }
}