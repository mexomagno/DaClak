# DaClak

Homemade wall clock. Just for fun. And an excuse to use some of the stuff I have lying around.

## Why?

I was tidying up my bedroom when I noticed lots of electronics stuff laying around collecting dust. So I though... what the heck, lets do something with this. So I decided to make a wall clock

## Features
* Accurate(ish) time and date
* Timezone support
* Bluetooth interface
* Scrolling text display

## TODO
[APP MISSING]- Configurable desde teléfono (Bluetooth) 
[OK]- Da la hora (accurate)
[OK]- Da la fecha
- Ahorro energía
- Funciona a pilas
- Vista de estado batería
- Despertador arduitunes
[OK]- Muestra mensajes random
- Colores?
- Configurable
	* Baudrate
	* Brightness
- Brillo acorde con luz ambiente
- Configuraciones persistentes

## Main parts

### The Display

Segments are controlled via some [shift registers](http://www.ti.com/lit/ds/symlink/sn74hc595.pdf).

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

On success, `\nOK\n` is returned. Else, `\nERROR\n`.

All commands and responses are text based.

Currently the following commands are available:

| Command | Meaning | Response |
| ------- | -------------------------- | ----------- |
| `TIME` (debug) | Get human readable date time via BT | Text with formatted date time |
| `DATE` | Display date in clock, and return date in human readable format via BT | Text with formatted date |
| `ST` | Set time with UTC unix timestamp in seconds | success response |
| `GT` | Get time as UTC unix timestamp | UTC timestamp in seconds |
| `SZ` + `X.Y` | Set timezone offset X.Y (float value) | success response |
| `GZ` | Get timezone offset | Timezone offset as a float |
| `TX` + `ABCD` | Show scrolling text "ABCD". Must be shorter than 253 characters | Success response |
| `SX` + `N` | Set text scroll shift delay in milliseconds. Move characters every N ms | Success response |
| `SD` + `N` | Display date for N milliseconds | Success response | 



## Issues
* El método Schedule requiere una funcion estática, pero el método de cpp para actualizar el texto que ahora estoy haciendo depende de la instancia!