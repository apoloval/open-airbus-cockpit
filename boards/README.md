# Open Airbus Cockpit - Electronic Boards

In this directory you'll find the electronic board designs used in Open Airbus
Cockpit. They are designed using [Fritzing](http://fritzing.org/home/), an 
open-source tool for circuit design. 

Each of the following directories contains the appropriate `.fzz` file and its 
companion `README.md` to document the circuit logic. 

* [8-Bits Expansion Card](8bits-expansion/): the 8-bits Expansion Card 
comprised by one input and one output 8-bits shift register. It makes possible 
to control 8 digital inputs and 8 digital outputs using only 6 pins of the 
Arduino board, ideal to manage buttons or switches with LEDs. It provides 4 
optional connectors for potentiometers up to the standard 10 pins conector. 
The 8-bits Expansion Cards can be stacked, connecting the shift registers in 
series to control 16, 24 or 32 inputs and outputs with the same 6 pins of the 
Arduino board. 

* [Keypad Expansion Card](keypad/): some parts of your cockpit cannot be 
easily handled using 8-Bits Expansion Card. Think about the MCDU, which has 
over 70 buttons. For this specific purpose of many buttons that act as keys
(they cannot be pressed simultaneously), it makes more sense to use a keypad
controller. That is the purpose of Keypad Expansion Card, which provides
a couple of MM74C922 chips, each one capable to manage a matrix up to 16 keys,
that can be stacked. You only need 4 digital inputs available in your Arduino
for the data bus plus 2 additional for each 74C922 chip to manage. A complete
MCDU can be controlled with only 3 Keypad Expansion Cards and 14 digital pins 
of your Arduino board. 
