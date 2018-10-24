//
// Created by mexomagno on 23-10-18.
//

#ifndef DACLAK_BTCONNECTION_H
#define DACLAK_BTCONNECTION_H
class BTConnection{
    /**
     * Designed to abstract communication between BT module and the arduino
     */
public:
    BTConnection(unsigned char rx_pin, unsigned char tx_pin, unsigned long baud_rate, *ClockDisplay);
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
#endif //DACLAK_BTCONNECTION_H
