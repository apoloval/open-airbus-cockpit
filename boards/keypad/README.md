# Open Airbus Cockpit - Keypad Expansion Board

## Introduction

This board provides a way to manage up to 32 keys using only 8 pins of the Arduino board. The boards can be chained supporting additional 32 keys using only 4 more digital pins. The purpose of this board is to manage cockpit devices as the MCDU, the ECAM panel or part of the radio panel. 
Anything that presents a significant amount of push buttons or keys that cannot be pressed simultaneously is a good candidate the be
managed with the Keypad Expansion Card. 

## Connectors

* **PWR**. A pair of pins aimed to provide 5v and GND power. 
* **PWR Link**. A pair of pins that are directly connected to the PWR terminals. They can be used to provide power to the next card in the stack by a cable that goes to the PWR connector. 
* **DATA BUS**. A bus comprised by 4 wires that is used to transmit the data from the keypad controllers to the Arduino board. The bus is used
simultaneously by all the controllers of all the stacked cards. Which one is transmitting which data is determined by the DAV and OE lines of each
controller. 
* **DATA BUS LINK**. These pins are used to connect the bus to the next card in the stack. The corresponding cable would be connected to the DATA
 BUS line of the next card. 
* **DAV**. Data available signal. When one of the controllers have something to transmit through the bus, this signal is high. Each controller has
one DAV pin.
* **OE**. Output enabled signal. This signal is activated by the Arduino board with inverted logic to indicate the controller it may use the data 
bus. Each controller has one OE pin.
* **OSC**. A port to connect a capacitor that acts as an oscillator for the controller. Each controller has one OSC capacitor. 
* **KEYB**. A port to connect a capacitor that manages the key bounce for the controller. Each controller has one KEYB capacitor. 
* **Key matrix pins**. A 4x4 matrix of 2-terminal connectors, each one aimed to connect one button. 

## Operation

Each card provides two 74C922 controllers, each one able to manage up to 16 keys. As mentioned above, the data bus is common to all the controllers
including those in stacked cards. When some key is pressed, the wiring of the matrix indicates to the controller what is the column and the row of
the key that was pressed. Then, the controller executes its logic to prevent key bouncings. Finally, if a key is finally detected as pressed, the DAV output of the corresponding controller is activated to high. This signal would be processed by the code of the Arduino board, and then it will
set the corresponding OE pin to LOW, which indicates to the controller it is able to use the data buffer. While the controller's OE signal is set to HIGH, the controller would dump the code of the pressed key to the data bus. The Arduino code would read that code from the bus and then it would
set the OE signal back to high. 
