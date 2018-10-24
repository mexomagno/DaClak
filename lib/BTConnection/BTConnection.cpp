//
// Created by mexomagno on 23-10-18.
//

#include "BTConnection.h"


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

/**
 * Checks if there's BT data to read, and processes it with a given callback.
 * @param callback
 */
void BTConnection::listen(bool callback(char *)) {
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
        bool result = callback(buffer);
        if (result){
            this->send("OK\n");
        } else {
            this->send("ERROR\n");
        }
    }
}

bool BTConnection::send(char *COMMAND) {
    this->bt_module->print(COMMAND);
}