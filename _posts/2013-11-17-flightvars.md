---
layout: post
title: "FlightVars"
description: ""
category: 
tags: 
---
{% include JB/setup %}

I think it's time to talk about FlightVars. 

One of the key points of the software architecture of Open Airbus Cockpit is how the electronic devices controlling switches, buttons, LEDs, displays, etc, communicate with the flight simulator. The hardware must have a mechanism in order to progress commands to the simulator and represent the state of the aircraft. 

There are several approaches to communicate your hardware with the flight sim. None of them are perfect. 

<!--more-->

One of the most popular is [FSUIPC](http://www.schiratti.com/dowson.html), which has become a defacto standard for Flight Simulator addon vendors. Its popularity is due to its simplicity. The whole status of the simulation is stored in a data buffer inside the sim. FSUIPC provides two primitives, `read()` and `write()`, to obtain or alter the state of the simulation, respectively. Each primitive operates on a given offset (a internal address in the buffer), so it is possible to read and write only certain parts of the sim state.

The FSUIPC architecture is pretty fine when an addon have to change the state of the simulation. It's a good candidate if all you want is to process commands that alter the aircraft state by writting into FSUIPC offsets. But if your addon have to observe the state of the simulation, FSUIPC is not the best solution. With only read and write primitives available, the only way to observe the simulation is to read FSUIPC offsets periodically and detect changes by comparing each result of read operation with the result of the previous call. Note that even if the datum has not changed we have to read it in order to be aware of any change. And we have to do it relatively often. The more frequently we read the less delay we will obtain from the instant the change happens and the instant we detect it. But it is obvious that we will waste more CPU (and perhaps network) resources. For an addon that have to observe a bunch of variables, that's not a big deal. But, can you imagine an addon that reads several hundred of variables several times per second? Of course you can: that's your cockpit. The FSUIPC approach, known as [polling](http://en.wikipedia.org/wiki/Polling_%28computer_science%29)), may become a performance killer for your cockpit. 

Other software component used to write FS addons is [SimConnect](http://msdn.microsoft.com/en-us/library/cc526983.aspx). SimConnect is an API shipped with MS Flight Simulator SDK, which provides functions to read and write variables and send or receive events. Both variables and events are identified by name rather than address or offset. The event system follows the [publish-subscribe](http://en.wikipedia.org/wiki/Publish_subscribe) pattern, so no more polling is needed in SimConnect! We can code an addon that observes the simulation by indicating what is the data it is interested in (subscribe). When any variable changes, the corresponding event is sent to the addon (publish). 

SimConnect could be a very good candidate for cockpit builders if (and only if) we ignore some considerations. Firstly, and not necessarily the most important, the API is terrible. As most of APIs written by Microsoft, it's very hard to use and understand. It implements most of the [anti-patterns of software engineering](http://en.wikipedia.org/wiki/Anti-pattern#Software_engineering). Secondly, and the most important one, any addon using SimConnect must run in the same machine as the simulator runs. And, of course, there is nothing like a SimConnect client for your favorite microcontroller. You have to implement some piece of software that interacts with your electronic devices and transmits the information to and from the simulator using SimConnect. Not funny. 

Well, there are other alternatives that we should mention. [IOCP Server](http://www.opencockpits.com/index.php/es/forum/iocp-server-and-other-iocp-aplications) is a very good candidate software that many people are using in their cockpits. It mixes the offset-oriented approach with a publish-subscribe mechanism across a TCP/IP network. An IOCP client connects to the server, running in a FS plugin, using TCP protocol. When connection is established, the client sends its subscriptions to the server by indicating the list of offsets it is interested in. After that, the server will respond with a variable-changed event every time that a variable changes. The client may send the same events as well to the server, indicating it wants to overwrite the current value of the variable. 

The main drawback of IOCP is that the offsets are bounded to certain data, and the offset map cannot be extended without rewritting the server. If you want to publish or subscribe to a data that is not mapped by IOCP server you'll have to think in another approach. And that's not so unusual. Many third-party aircrafts do not use the standard variables to maintain the state of the aircraft.

We have seen several approaches, and as I introduced early, none of them are perfect for the job. That's weird. Integrating electronic devices with a flight simulator software is not brain surgery. We should have a software component that fits perfectly to our needs as cockpit builders. We deserve it!

Now, it's time to talk about FlightVars. Imagine a FS plugin that runs a mechanism that allows you to publish-subscribe to a variable by name. What kind of variable? Potentially, any of them. Some examples of variables could be:

* `/flightvars/fsuipc/offset/0x1234:2`, which names the FSUIPC offset `0x1234` read as a 2-bytes value.
* `/flightvars/simconnect/var/Indicated Altitude:feet`, which names the SimConnect variable `Indicated Altitude` as a value represented in feet.
* `/flightvars/projectmagenta/a320/var/apu_gen_status`, which names a Project Magenta variable corresponding to the APU generator status of a A320 aircraft. 

Now imagine that your addon can subscribe to any of these variables, and the plugin is able to publish any change when it happens. Imagine that your addon can publish as well any change it wants to in order to overwrite the current status of the variable. It means that you could publish-subscribe to FSUIPC offsets, SimConnect variables or internal data exported by any third-party aircraft addon using that imaginary plugin. Imagine that you can easily use connect to that plugin using an intuitive API already available in many programming languages and many microcontroller platforms. Imagine as well that extending the plugin is an easy task, you can add more namespaces to the ones that are shipped with the product. An finally, imagine this software is free or charge and open source. 

Can you image all that? Congrats! You are imagining a reallity: __OAC FlightVars__. Want to know more? Great! Keep this frequency. 

Happy flying!
