---
layout: page
title: Electronics
group: navigation
permalink: hardware/electronics.html
section:
  - { title: "Hardware", url: "/hardware.html" }
---

{% include JB/setup %}

* Table of Contents
{:toc}

Most parts of the cockpit are electronically controlled in order to receive 
the input of the pilot and show the responses and the status of the avionic 
systems. Each button, switch, key, rotary, display, LED indicator, etc, is 
managed by a electronic device. 

Open Airbus Cockpit electronics is based on Arduino platform. Arduino is, 
according to their website:

_Arduino is an open-source electronics prototyping platform based on 
flexible, easy-to-use hardware and software. It's intended for artists, 
designers, hobbyists and anyone interested in creating interactive objects 
or environments._

Most of cockpit builders are using [IOCards](http://www.opencockpits.com/index.php/en/iocards/item/Description?category_id=63), but during the early stage 
of Open Airbus Cockpit project this solution was discarded in favor of 
Arduino. The main reason is that IOCards is a ad-hoc solution for cockpit 
builders. Perhaps it sounds as a pro instead of cons, but it is not. Consider 
that there are only out there some hundreds, a few thousands at most, of 
IOCard units. That means poor support, poor documentation, poor training, etc. 
If we compare that with the number of Arduino units distributed all over the 
world and its community of users, we can be sure that the tons of 
documentation, howtos, guides, books, references, etc, are far beyond the most 
prolific IOCards community. 

One of the main drawbacks of Arduino platform is that, as with any other 
microcontroller architecture, the logic of your electronics must be programmed. 
It means writing code. That's something terrorific for many cockpit builders, 
but not for me (I'm a professional programmer). That makes the Arduino platform 
not only inoffensive but even a beautiful thing as it can be programmed in C++. 
If you are not so lucky to be a skilled programmer, don't worry. Please remember 
that Open Airbus Cockpit is open, and all the source code that runs on the 
Arduino board is open sourced and available in 
[GitHub](http://github.com/apoloval/open-airbus-cockpit/arduino). 

## Arduino Overview

<img class="image-right" 
     src="http://upload.wikimedia.org/wikipedia/commons/3/38/Arduino_Uno_-_R3.jpg"
     style="width: 200px;" />

Perhaps the Arduino description provided above is meningless for most
of the readers. Think in Arduino as a microcontroller with awesome capabilities
to interact with its environment. 

Each Arduino board provides a set of pins to perform digital input and output. 
Let's say you are able to read and write digital signals using its pins. An 
input signal may be a push button that has been pressed or a switch that has 
been turned on. An output signal may be used to power a LED or to show a 
number in a 7-segment display. 

Along its digital pins, it provides also analog pins. The analog pins can be used
to read the voltage connected to that pin. This, combined with potentiometers,
allow you to connect rotary controls that can be read from the Arduino board.

Well, so we can connect almost anything to the Arduino board. And what's the
point? Of course, this is useless until we introduce some logic to the 
microcontroller to know what to do with the inputs and the outputs. You can 
do that by providing a program to the Arduino board that reacts against its 
inputs by possibly altering its outputs. E.g., your program can read the current
value from a digital input pin that has a push button connected. When pressed,
the digital pin would pass to 5v so the digital circuits would detect a _high_
logical value. Your program would detect that value and it could react against
it by powering a LED connected to one digital output. 

And, how do I put all that logic into a program? Using the Arduino programming
language. Something similar to:

<div class="code"><pre><code class="cpp">
int buttonPin = 12;
int ledPin = 13;

void setup() {
	pinMode(buttonPin, INPUT);
	pinMode(ledPin, OUTPUT);
}

void loop() {
	int isPressed = digitalRead(buttonPin);
	if (isPressed == HIGH) {
		digitalWrite(ledPin, HIGH);
	} else {
		digitalWrite(ledPin, LOW);
	}
}
</code></pre></div>

<img class="image-right image-framed"
     src="http://upload.wikimedia.org/wikipedia/commons/d/d4/Arduino_Code_Editor_with_5DOF_Sensor_Code.png" 
     alt="Arduino IDE" 
     style="width: 350px;"/>

It's not so difficult, isn't it? If you are not familiar with programming 
forget about the syntax right now. You'll have time to learn all the details 
later. Just think about the relevant sections of the code. 

* Firstly, we declare some constants called `buttonPin` and `ledPin`. We 
initialize them to the number of the pins out button and out LED are connected, 
respectively.
* Then, we declare how to setup the program, i.e. how to initialize the 
system. There we just indicate the mode of each pin, indicating we expect
inputs from the button and outputs to the LED.
* Finally, we define the program loop. This section will be executed
after the setup section and will do that continuously until the Arduino board 
is powered off. Here continuously means that the program will run the code
of the loop section over and over again. Here is where the logic of our
program goes. In this case, it reads the state of the button and power on
or off the LED in case the button is pressed or unpressed, respectivelly. 

Ok, that's the code. And how do I put it into the Arduino microcontroller?
Arduino Project ships a free and open source tool suite that includes a
Integrated Development Environment (IDE), i.e. a tool to write code and,
once your program is completed and correct, upload it into an Arduino board 
connected an USB cable to your PC. 

For more details on how to getting start with Arduino platform, please
check out the learning resources in 
[Arduino website](http://arduino.cc/en/Guide/HomePage).

## Arduino and Open Airbus Cockpit

The most basic approach to use Arduino in your cockpit would be quite simple. 
We'd have one or more Arduino boards connected to the PC where your simulator
runs using USB cables. Then, the electronic components as buttons, switches, 
rotaries or LEDs are connected to the pins of one Arduino board. The code of
your Arduino sketch would interact with the simulator using some communication
facility and would execute something like:

<div class="code"><pre><code class="cpp">
void loop() {
	stateChange = readStateChangeFromSimulator();
	if (stateChange != null) {
		writeStateChangeToPanel(state);
	}

	action = readActionFromPanel();
	if (action != null) {
		writeActionToSimulator(action);
	}
}
</code></pre></div>

This is obviously a simplification, but may help to understand how it works. 
Firstly, we could try to obtain (a change in) the state from the simulator.
If there is any, we would write it to the electronic components attached to 
the Arduino outputs. After that, we would read some action (if any) from the 
input components and would transmit that action to the simulator. This is part
of the program loop of your sketch, and would be repeated forever until your
Arduino is powered off. 

Let's see a short example. Let's say we have an Arduino board that has a
group of 7-segment displays and a couple of push buttons attached to its pins.
The purpose of these elements is to show and manage the QNH of the aircraft.
The displays are meant to show the QNH number, and the two buttons are used
to increase or decrease the QNH value. 

With the code template we've seen above, the Arduino board would check
if there is some change in the state provided by the Simulator (i.e., the QNH
has changed). If so, it would write the new QNH number to the 7-segment 
displays. If there is no change, it wouldn't do anything. After that, it would
check whether any of the buttons is pressed. If so, it would send the 
appropriate command to the simulator (increase or decrease the QNH). If no
button is pressed, it does nothing. 

Unfortunately, the things are a more complex than that. But that's the basic
mode of operation of the Arduino platform in your cockpit. Across the following
sections we would provide all the details you have to know to be able to build
your cockpit hardware using OAC technology. 

## Expansion Cards & Auxiliary Controllers

<img class="image-right"
     src="http://upload.wikimedia.org/wikipedia/commons/thumb/8/80/Three_IC_circuit_chips.JPG/295px-Three_IC_circuit_chips.JPG" 
     style="width: 200px;"/>

As we mentioned before, this is all about connecting electronic components to
the pins of your Arduino board. Each button or switch to one digital input pin. 
But, how many buttons and switches do we have in an airline cockpit? Just 
think about the MCDU. It has over 70 buttons! And outputs are no better. Each 
LED is connected to one digital output pin. Each segment of a 7-segment display 
to one pin, up to 7 pins per digit (8 if we consider the dot). A group of 
displays of 6 units requires from 42 to 48 pins. Now think how many pins would
require the A320 FCU for all its displays. 

I'm sure you got it. An airline cockpit has too many inputs and outputs to 
simply connect all them to Arduino boards. We would require dozens of them.
Fortunately, we have some tricks to avoid such a situation. 

Most of the interactions we want to make with the electronic components are
typical. So typical that many of people had to solve it before you. And
they realiced that it was simpler to encapsulte that single interactions into 
integrated circuits (IC) to compose a more complex interaction. For instance, 
a set of push buttons can be combined into a matrix. There are some ICs
in the market that can be used to interact with that matrix, so the IC is
able to indicate what button was pressed using just a few digital lines. 
In this particular case, the buttons of your MCDU could be managed with
14 digital pins of your Arduino board. If we compare that with using over
70 pins, one per button, we can see the clear benefit of this approach. 

It is highly recommended to use this kind of ICs (sometimes known as 
_drivers_) to simplify the interactions saving Arduino board pins. OAC project
provides some generic cards designed for that purpose, as we'll see below. 

### 8-Bits Expansion Card

This board provides a way to have 8 digital inputs and 8 digital outputs 
by using only 6 IO pins of the Arduino board. The boards can be chained, 
providing 8 more digital inputs and 8 more outputs per each chained board 
using the same number of pins in the Arduino side. In addition, each board 
supports up to 4 analog input connectors that may be used to connect 
potentiometers to the Arduino board, and an auxiliary connector that provides 
12v power for backlighting with LED strips.

### Keypad Expansion Card

This board provides a way to manage up to 32 keys using only 8 pins of 
the Arduino board. The boards can be chained supporting additional 32 keys 
using only 4 more digital pins. The purpose of this board is to manage 
cockpit devices as the MCDU, the ECAM panel or part of the radio panel. 
Anything that presents a significant amount of push buttons or keys that 
cannot be pressed simultaneously is a good candidate the be managed with 
the Keypad Expansion Card.

## Communicating to the Simulator

Once you have addressed how to interact with the electronic components, 
you'll find a task that is even more difficult to achive: how to interact
with the simulator.

There are several solutions that cover this topic you might know: FSUIPC,
SimConnect, IOCP Server, etc. All they are software products that exports 
the data from the simulated scene outside the simulator program so it can be 
used by other software, including those running in your hardware. 
Unfortunately, none of them are supported in Arduino platform and, even if
they were, they would require more sophisticated communication devices
as Ethernet shields to communicate using TCP/IP. 

To cover this gap, Open Airbus Cockpit provides a software component
known as OAC Command Gateway. This is a Lua script that runs on top of 
FSUIPC that is able to interface to any device using a very simple protocol
over the serial port. This is perfect for Arduino, which may listen to 
the simulator data and write to it using a library specifically written
for that purpose. 

To know more about Command Gateway and Arduino, please check the 
[corresponding software section](../software/command-gateway.html).


