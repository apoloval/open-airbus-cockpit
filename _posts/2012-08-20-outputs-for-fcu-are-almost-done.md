---
layout: post
title: "Outputs for FCU are almost done"
category: Hardware
tags: [fcu]
---

When I started working on OAC, I had one thing clear in my mind: the first device to implement should be the Flight Control Unit. Why? Because it is complex enough to have an idea of the effort required and simple enough to built it with commodity parts (e.g., no display screen is required as for FMGS). 

So I started with FCU. Breadboards, 7-segment displays, integrated circuits, rotary encoders, buttons, and cables, dozens of cables. And, of course, the Arduino board to rule them all as Sauron meant with the Middle-Earth. The first code I wrote was in the latter of July in Foz, a beautiful village in the north-west of Spain. My vacations, and I didn't hesitate to pack my hardware parts and laptop in the case. Among relaxing sessions in the beach and delicious meals, I mounted the breadboard and programmed some code.

<!--more-->

The device is comprised by sixteen 7-segment displays. Before purchasing the parts I realize I would have a problem with the number of output pins of the Arduino board. Each display requires 8 pins (7 segments plus dot point). That means 144 pins only for displaying numbers. That's insane, and obviously there are some tricks to save pins. 

One alternative is to use serial shift registers like the 74HC595 chip. This integrated circuits have a sort of internal memory and are able to maintain a determined digital value for their outputs. The nice point is that they are managed via serial communication. Only three pins are required to manage 8 outputs. Or even more, since the 74HC595 may be chained in cascade providing 8 more outputs per additional chip. 

This is the cheapest alternative, but not the simples one. Using shift registers means programming the display logic by yourself. In other words, any number or symbol displayed is calculated by your code executed on the Arduino board. Another cons is that you will need one resistor per segment to keep the current flow in the range indicated by the manufacturer. There is another alternative, which simplifies the things by requiring more output pins of your Arduino board: the ICM7218.

The ICM7218 integrated circuit is designed to manage up to 7-segment displays  using a very simple communication protocol with the microcontroller. It takes care of the current flow, so no resistors are needed. It takes care of the logic of displays, so you provide numbers which it decodes to the proper segment logic. The cons is that it takes 10 output pins to manage it. 

So finally I decided to use ICM7218 rather than shift registers. So for 16 displays I needed a couple of chips, each one consuming 10 output pins of Arduino board, with a total of 20 pins dedicated to 7-segment displays. This immediately discards the most popular and cheapest Arduino board, the Arduino Uno (which only have 14 digital pins), in favor or Arduino Mega 2560 (which have the impressive amount of 54 digital pins). 

Along 7-segment displays, the FCU requires some LED bars to indicate the active modes. These are indicators present in the FCU display like SPEED/MACH mode, HDG/TRACK mode, managed modes engaged..., and so on. Anyone familiarized with Airbus operation knows what I mean. This bars are wired among them when necessary. For instance, HDG and TRACK indicators cannot be active at the same time, since each one means the opposite respect the other. In addition, some modes cannot be independently controlled. For instance, the HDG/TRACK and the VS/FPA are wired, so if HDG mode is enabled, VS is enabled as well. Changing from HDG to TRACK means changing from VS to FPA. This wiring saves some output pins of the Arduino board, since only one PIN is required to manage the HDG, TRACK, VS and FPA indicators. 

Not much more to say about FCU so far. I succeed to integrate it with the test utility, a desktop application that simulates the status of the FCU using a Windows form. As soon as I have it all bounded, I will upload some videos of the FCU prototype working with the test utility.

Happy flying!
