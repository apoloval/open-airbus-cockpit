---
layout: page
title: Computers
group: navigation
permalink: hardware/computers.html
section:
  - { title: "Hardware", url: "/hardware.html" }
---

{% include JB/setup %}

This is a trivial observation, but you will need at least one computer in your
cockpit. After all, you are gonna make a simulator. And, until a better 
invention to simulate the reality goes into our lifes, computers are a 
essential part of the process.

## How Many Computers?

It is not unusual at all to see some cockpit projects with almost a data center
installed in their cockpits. That is not necessarily bad, but you should have
a good reason before using more than a single high-end PC for your simulator.

There are some historical reasons to explain why some people believe they
need so many computers. This is a hobbie that dates back to the late 90's. 
With Intel Pentium processors just released, the capacity of the computers
was far away from what we are custom today. With this very limited hardware
profiles, the simulator itself had to be very optimized to provide a simulation
experience that was still pretty limited. In that time, adding more tasks to
run in the same PC meant overwhelming its capacity. Additional computers were
needed to execute the supporting software and preventing it to exhaust the
main computer resources. Also the graphics card of that time didn't support
multiple displays, what is a must for an airliner cockpit. 

Many years have passed since then. The CPU clocks have been increased from
100Mhz to over 4Ghz. The main memory passed from dozens of megabytes to several
gigabytes. Real parallelism came into processors so it's not unusual to have 4
cores with hyperthreading support. The speed of the memory and the rest of
circuitry of the motherboards has been dramatically increased. Yes, the
simulators are more sophisticated as well, indeed. But even so the resources of
a PC running FSX or Prepar3D are underused. Modern graphic cards also supports
multiple displays simultaneously, so that's not a constraint either. If we
consider also that interfacing software does not demand high computing
resources, using more than one PC for your cockpit might be completely
pointless.

## Hardware profile

You probably do not need more than one PC, but not *any* PC. 

It is highly recommended not to use a laptop or a barebone. Yes, they are small
and can be easily racked in some corner of the cockpit occuping little space.
But you are not gonna chat, browse and write docs with it. And that's what
laptops were designed for. You are going to execute a simulator that would
require a considering amount of CPU cycles and will lead your graphics card to
its limit. If you don't want to see how your laptop melts in final approach,
consider adquiring a PC.

PC? Not Mac? Well, that depends on the software you will use. Open Airbus
Cockpit uses Microsoft Windows with Prepar3D, so a PC is a good choice. Of
course, you can use a Mac with Mac OS X and X-Plane if that's the software you
have chosen. Nevertheless, the software of Open Airbus Cockpit doesn't support
that setup.

And what would be the hardware profile of that PC? Well, there is one computing
resource  your simulator will demand the most: 3D acceleration. Make sure you
have a high-end graphics card. Brand? Up to you. Some people recommend NVidia,
some other ATI. That's your choice but make sure you get one with a performance
enough for gaming. Don't use a low-end, motherboard-embeded Intel graphics card
or similar unless you want to flight below 10 FPS.

Another relevant consideration would be the number of display outputs the
graphics card supports. Depending on your visual setup, you may need several
displays for the visual. Also depending on how do you implement your glass
cockpit and its displays, you may need additional ports in your graphics card.
Make sure your graphic cards supports as many displays as you'll need.

What about CPU? Any modern processor with 2 or 4 cores would be enough. Memory? 
FSX/Prepar3D consumes from 2 to 4 GB of memory. 8GB is more than enough. SSD
is useful if you want to reduce the launching time of the simulator and may
have some impact while loading textures from disk. But that's not strictly
necessary.

So that's more or less all you need. It's important to know that surpassing 
this specs doesn't mean you'll have a better simulation experience. A 16 cores
processor would be idle most of the time and would have similar results to
one of 4 cores. Additional memory won't be used at all by your sim, as a 
larger hard disk won't make it faster. Only your graphics card could be a 
limiter to your framerate. In this sense, it could be interesting to 
have a motherboard that supports more than one graphics card so you can
stack them to increase its performance. Search the web and read more about
the best profile for your simulator. It would be applicable to your cockpit
as is since as it was mentioned above the rest of the software would consume
an insignificant amount of resources compared to the simulator.
