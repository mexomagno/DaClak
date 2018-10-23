# DaClak

Homemade wall clock. Just for fun. And an excuse to use some of the stuff I have lying around.

## How it works

### Bluetooth module:
This uses an HC-06 bluetooth module.

Reccomended setup:
* VCC to 5V
* GND to GND
* TX to arduino RX port (any port set to be RX)
* RX to arduino TX through voltage divider (down to 3.3v)

Relevant info can be seen [here](http://www.martyncurrey.com/arduino-and-hc-06-zs-040/)

More info:
| Property | Value |
| -------- | ----- |
| FW Version | linvorV1.8 |
| name | DaClak |
| pin | the one for wifi |


## BT Protocol

Every command ends with `\nOK\n` if everything went well.
All commands and responses are text based.

Possible actions:

| Action | Command | Expected response |
| ------ | ------- | ----------------- |
| Get human readable time (debug) | `TIME` | The time in HH:MM:SS. If timezone is defined, it includes it too |
| Get human readable date (debug) | `DATE` | The date in dd/mm/YY format. If timezone defined, included too |
| Set unix timestamp | `STXXXXXXXXXX` | Confirmation. It should set a specific unix timestamp |
| Get unix timestamp | `GT` | UTC Unix timestamp in seconds |
| Set timezone offset | `SZX.Y` | Confirmation. Set timezone offset. Accepts floats, positive and negative |
| Get timezone offset | `GZ` | Timezone float |





## Features

## TODO
[APP MISSING]- Configurable desde teléfono (Bluetooth) 
[OK]- Da la hora (accurate)
[OK]- Da la fecha
- Ahorro energía
- Funciona a pilas
- Vista de estado batería
- Despertador arduitunes
- Muestra mensajes random
- Colores?
- Configurable
	* Baudrate
	* Brightness
	* 
- Brillo acorde con luz ambiente
- 