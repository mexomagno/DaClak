//
// Created by mexomagno on 23-10-18.
//

#ifndef DACLAK_BTCONNECTION_H
#define DACLAK_BTCONNECTION_H

#include <SoftwareSerial.h>
#include <Arduino.h>

class BTConnection{
    /**
     * Designed to abstract communication between BT module and the arduino
     */
public:
    BTConnection(unsigned char rx_pin, unsigned char tx_pin, unsigned long baud_rate);
    void begin();
    bool send(char COMMAND[]);
    void listen(bool method(char *));
private:
    unsigned char rx_pin;
    unsigned char tx_pin;
    unsigned long baud_rate;
    SoftwareSerial *bt_module;
    char buffer[128];
};
#endif //DACLAK_BTCONNECTION_H
