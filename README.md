# DaClak

My own handmade clock & screen. Just for fun. And an excuse to use the electronic junk I have lying around.

## Why?

I was tidying up my bedroom when I noticed lots of electronics stuff laying around collecting dust. So I though... what the heck, lets do something with this. DaClack was born.

## Features
* Accurate(ish) time and date
* Timezone support
* Scrolling text display
* Bluetooth interface
  * Get/set time
  * Get/set date
  * Get/set timezone
  * Display a message
  * Message scroll settings

## Main parts

### The Display

The display supports 6 14-segment digits (84 total segments).
Each digit is turned on 1/6 of the time and enabled by digital outputs 8 to 14 (Arduino Uno's `PORTB`)

Two `SN74HC595N` [shift registers](http://www.ti.com/lit/ds/symlink/sn74hc595.pdf) in series output 16 bits, from which 14 are used
for the digit segments, and 2 for the second indicators

### Bluetooth module:
This uses an [HC-06 bluetooth module](http://www.martyncurrey.com/arduino-and-hc-06-zs-040/).

Recommended setup:
* VCC to 5V
* GND to GND
* TX to arduino RX port (any port set to be RX)
* RX to arduino TX through a 3.3v voltage divider (R1 ~ 2R2)

## BT Protocol

Communication protocol is text based. 

Supported commands:

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

For each command, the microcontroller returns `\nOK\n` on success, and `\nERROR\n` on failure.

## TODO

[OK]- Configurable desde teléfono (Bluetooth) 
[OK]- Da la hora (accurate)
[OK]- Da la fecha
[OK]- Muestra mensajes random
- Mobile app para interfacear
- Fix brillo puntos (no prender en los 6 ciclos)

## Brainstorm

- Ahorro energía
- Funciona a pilas
- Vista de estado batería
- Despertador arduitunes
- Configurable
	* Brightness
- Brillo acorde con luz ambiente
- Configuraciones persistentes
- Buzzer


## Issues
* El método Schedule requiere una funcion estática, pero el método de cpp para actualizar el texto que ahora estoy haciendo depende de la instancia!

## Relevant for my particular implementation

| Property | Value |
| -------- | ----- |
| FW Version | linvorV1.8 |
| name | DaClak |
| pin | `eldepa wifi number` |
