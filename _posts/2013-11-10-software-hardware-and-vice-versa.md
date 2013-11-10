---
layout: post
title: "Software, Hardware and vice versa"
description: ""
category: 
tags: 
---
{% include JB/setup %}

These last weeks have been plenty of software and plenty of hardware.

Let's start talking about hardware. After several weeks flying with only the left MIP panel, I have decided to work on the rest of MIP panels. I used the same technique used for captain MIP: design the panel with [Librecad](http://librecad.org/cms/home.html), draw it onto a 3mm wood panel and cutting it using a jigsaw and a drill for the round corners. After some paint, the result is similar to the following one.

<div class="pics">
<p>
<img src="https://s3-eu-west-1.amazonaws.com/open-airbus-cockpit/pics/IMG_0621.JPG" style="width: 380px;" align="left" />

<img src="https://s3-eu-west-1.amazonaws.com/open-airbus-cockpit/pics/IMG_0632.JPG" style="width: 380px;" align="right" />
</p>

</div>

Work in progress, of course. I am considering the chance to cut the panels using a laser cutting machine to produce more precise shapes. But that's an story for another post. 

Now, let's talk about software.

<!--more-->

So far, the goal of OAC software have been to integrate the electronic components with a FS addon aircraft like Wilco A320 or Airbus X. The reason to have defined that goal is simplicity. Use an available software and reduce the task only to integrate the cockpit state with the electronics outside the simulator. 

Unfortunately, that approach is not as simple as it initially seemed. The authors of Wilco Airbus and Airbus X were not thinking in cockpit building when they implemented their products. For instance, Aerosoft Airbus X is not designed to output the glass cockpit displays in separate panels. You may use a [known hack](http://forum.aerosoft.com/index.php?/topic/65608-how-to-set-up-a-system-with-4-monitors-wits-airbus-x-extended/#entry481237), but you still will have issues with ND display. On the other hand, export the internal state of the aircraft to be handled by electronic devices is not trivial. As discussed before, Wilco Airbus doesn't provide any API to export such data. Airbus X provides some information using SimConnect variables, but under the risk to be mistaken I guess not all of them are available (checking it is in my TODO list). 

Another important drawback of using these third-party addons is that the aircraft systems are not simulated very well. As an example, let's jump into a cold and dark Airbus X cockpit and try to connect the APU without batteries or ground power connected. That shouldn't work at all. The APU starter is energized by the DC battery bus, which is not powered with the batteries and the external power disconnected. But, after several seconds, you'll check that the APU has started and then all the systems are magically energized. 

Well, after all, using third-party aircrafts is not a good idea after all. Alternatives? 

* We may pay thousand of euros for a [Project Magenta](http://www.projectmagenta.com) license. But that goes against the low-cost principle of OAC project. 
* We may wait for a new addon designed for cockpit builders. An addon with a complete SDK to integrate the simulated systems with the electronic devices. An addon with aircraft systems simulated with high fidelity. Wait... how much time?
* We may implement our own addon, designed for cockpit builders, with a rich API, with realisticly simulated systems. 

I think I just revealed a proof-of-concept subproject of OAC codenamed Gwaihir. I will tell you more in upcoming posts. 
