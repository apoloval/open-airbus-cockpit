# Open Airbus Cockpit Serial Protocol

## Introduction

This library provides a wrapper to send and receive messages according
to OAC Serial Protocol. It can be used to communicate the Arduino board
with the flight simulator through OAC Command Gateway. 

## Getting Started

### Installation

**Important note**: OACSP requires OAC Command Gateway to work. You may
find intructions on how to install this component in [Command Gateway](
../../../cmd-gw/) README file.

Just copy this directory to the 'libraries/' directory of your Arduino 
installation (e.g., `%HOMEPATH\My Documents\Arduino\libraries`). After that,
the library will be available in Arduino IDE throught the menu entry 
_Sketch -> Import Library -> Arduino_ or simply including it in your code.

```c++
#include <oacsp.h>
```

### Initialization

OACSP must be initialized before any command is sent or received. Its as
simple as

```c++
void setup() {
	OACSP.begin("MyCockpit");
}
```

Where `"MyCockpit"` is the name that identifies your client in Command Gateway.
You may use any other name as long as it's not used by another OACSP device. 

### Write Offsets

With OACSP initialized, you may write into FSUIPC offsets as follows.

```c++
int pressed = 0;
void loop() {
	int wasPressed = pressed;
	pressed = digitalRead(BUTTON);
	if (wasPressed != pressed && pressed) {
		OACSP.writeOffset(0x0330, word(1013 * 16));
	}
}
```

In this example, once the button connected to `BUTTON` pin is pressed, the
value _1013*16_ is written into FSUIPC offset _0x0330_. The type of the
second argument determines how the value is written into the offset.

* A `byte` or `unsigned char` value is written as a 8-bit unsigned integer 
(_UB_ in FSUIPC terminology).
* A `char` value is written as a 8-bit signed integer (_SB_ in FSUIPC 
terminology).
* A `word` or `unsigned int` value is written as a 16-bit unsigned integer 
(_UW_ in FSUIPC terminology).
* A `int` value is written as a 16-bit signed integer (_SW_ in FSUIPC terminology).
* A `unsigned long` value is written as 32-bit unsigned integer (_UD_ in 
FSUIPC terminology).
* A `long` value is written as 32-bit signed integer (_SD_ in FSUIPC 
terminology).

Other types as float, double or strings are not supported so far. 

### Write LVARs

OACSP allows your sketch to write into LVARs (local variables managed by the
gauges or the cockpit of the aircraft). That is very similar as writting FSUIPC 
offsets. 

```c++
int pressed = 0;
void loop() {
	int wasPressed = pressed;
	pressed = digitalRead(BUTTON);
	if (wasPressed != pressed) {
		OACSP.writeLVar("AB_MPL_FD", pressed);
	}
}
```

In this example, each time the button changes the LVAR `AB_MPL_FD` is written
with the value of the button. Any integer value may be passed as argument.
Float or double values are still not supported. 

### Observe Offsets

OACSP is able to request the Command Gateway to start observing an offset
in order to receive a event each time its value changes. 

```c++
void setup() {
	OACSP.begin("MyCockpit");
	OACSP.observeOffset(0x0330, oac::OFFSET_UINT16);
}

```

This line in `setup()` indicates Command Gateway that it should observe
any change on the unsigned 16-bits integer at offset `0x0330`. The type
of the value that is observed is determined by the second argument passing
one of the following values.

* `OFFSET_UINT8` for a unsigned 8-bits integer (_UB_ in FSUIPC terminology)
* `OFFSET_SINT8` for a signed 8-bits integer (_SB_ in FSUIPC terminology)
* `OFFSET_UINT16` for a unsigned 16-bits integer (_UW_ in FSUIPC terminology)
* `OFFSET_SINT16` for a signed 16-bits integer (_SW_ in FSUIPC terminology)
* `OFFSET_UINT32` for a unsigned 32-bits integer (_UD_ in FSUIPC terminology)
* `OFFSET_SINT32` for a signed 32-bits integer (_SD_ in FSUIPC terminology)

Float, double and string values are still not supported. 

When a change is detected, a event will be sent to the Arduino board that can 
be read as follows.

```c++
void loop() {
	oac::Event event;
	if (OACSP.readEvent(event)) {
		switch (event.type) {
			case oac::OFFSET_UPDATE:
				if (event.offset.address == 0x0330) {
					showQnh(event.offset.value);
				}
				break;
		}
	}
}
```

In this example, the `loop()` function will retrieve an event from OACSP. If
there is no event awaiting, the call to `OACSP.readEvent()` will return _false_.
If there is an event, it returns _true_ and the `event` passed as argument
is filled with the information processed by OACSP. Then, `event.type` indicates
what kind of event has been received. In case of `oac::OFFSET_UPDATE` we
know that's a change on an OFFSET. We may look at `event.offset.address` to
know what's the offset that was changed. In case that's _0x0330_, we know
the QNH has changed in the simulator. Then we invoke a function `showQnh()`
that would show the new QNH value, stored in `event.offset.value`, to a 
7-segment display array. Please note that this `showQnh()` function is not
provided by OACSP but implemented by yourself. It's just an example of what
we could do by observing offsets. 

### Observe LVARs

We may observe LVARs as well using a mechanism similar to the offset 
observation.

```c++
void setup() {
	OACSP.begin("MyCockpit");
	OACSP.observeLVar("AB_MPL_FD");
}

```

We just invoke `OACSP.observeLVar()` with the name of the LVAR we want to 
observe. Then, after that LVAR is modified, a new event will arrive.

```c++
void loop() {
	oac::Event event;
	if (OACSP.readEvent(event)) {
		switch (event.type) {
			case oac::OFFSET_UPDATE:
				if (event.offset.address == 0x0330) {
					showQnh(event.offset.value);
				}
				break;
			case oac::LVAR_UPDATE:
				if (strcmp(event.lvar.name, "AB_MPL_FD") == 0) {
					digitalWrite(FD_LED, event.lvar.value);
				}
				break;
		}
	}
}

```

In this case, we add a new _case_ branch for `oac::LVAR_UPDATE`, which is
the value of `event.type` that indicate the event represents a LVAR update. 
Then, we can retrieve the name of the LVAR that was modified from 
`event.lvar.name`, and the new value of the LVAR from `event.lvar.value`. In
this example, we write in the digital output pin `FD_LED` the status of the
FD obtained from the Command Gateway. 

