# Open Airbus Cockpit - Electronic Boards

In this directory you'll find the electronic board designs used in Open Airbus
Cockpit. They are designed using [Fritzing](http://fritzing.org/home/), an 
open-source tool for circuit design. 

Each of the following directories contains the appropriate `.fzz` file and its 
companion `README.md` to document the circuit logic. 

* [8-Bits Expansion Card](./8bit-expansion/): the 8-bits Expansion Card 
comprised by one input and one output 8-bits shift register. It makes possible 
to control 8 digital inputs and 8 digital outputs using only 6 pins of the 
Arduino board, ideal to manage buttons or switches with LEDs. It provides 4 
optional connectors for potentiometers up to the standard 10 pins conector. 
The 8-bits Expansion Cards can be stacked, connecting the shift registers in 
series to control 16, 24 or 32 inputs and outputs with the same 6 pins of the 
Arduino board. 

