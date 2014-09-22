# Open Airbus Cockpit - 8-Bits Expansion Board

## Introduction

This board provides a way to have 8 digital inputs and 8 digital outputs by using only 6 IO pins of the Arduino board. The boards can be chained, providing 8 more digital inputs and 8 more outputs per each chained board using the same number of pins in the Arduino side. In addition, each board supports up to 4 analog input connectors that may be used to connect potentiometers to the Arduino board, and an auxiliary connector that provides 12v power for backlighting with LED strips. 

## Connectors

* **PWR**. This JST connector provides 12v, 5v and GND pins. The 12v line is only needed if you connect a LED strip backlight to the BLT port. 
* **DATA IO**. This double row 10-pins connector communicates the input and output data to and from the Arduino board.
  * Pin 1 is the data input clock signal 
  * Pin 2 is the data input latch signal
  * Pin 3 is the serial data input line
  * Pin 4 is the data output clock signal 
  * Pin 5 is the data output latch signal
  * Pin 6 is the serial data output line
  * Pins 7 to 10 are the analog 0 to 3 data input signals, respectively
* **S-INPUT**. This connector may be used to chain the input of another 8-bits Expansion Board serially. A 3-line cable connects from this port in the head of the chain to the pins 1-3 of the DATA IO of the next board in the chain. 
* **S-OUTPUT**. This connector may be used to chain the output of another 8-bits Expansion Board serially. A 3-line cable connects from this port in the head of the chain to the pins 4-6 of the DATA IO of the next board in the chain. 
