---
layout: post
title: "Reverse Engineering On Wilco Airbus"
category: Software
tags: [wilco]
---

We have been several weeks without news on OAC. But we have been working on it after all. The main front-lines have been the integration between OAC server and Karen Core framework (some utility code as a baseline for C++ programming) and the modeling of cockpit parts (both pieces as knobs and buttons to be 3-D printed and main cockpit structure). But that's content for future posts. 

Today I'm gonna write about one of the parts of the project that scared me most: the integration between OAC server and Wilco Airbus software. Wilco Airbus is the most realistic, payware Airbus available for FSX/Prepar3D. Unfortunately, it doesn't provide any SDK to aid cockpit builders. But fortunately, we have something called reverse engineering. 

<!--more-->

For those who are not familiarized with such stuff, reverse engineering is a discipline aimed to discover the technological principles of something from its implementation. In case of Wilco Airbus, we are interested in finding out how to interact with it from — its compiled code. It doesn't look easy, and it is not. But impossible is nothing. Actually, I'm not the first guy who tries. Eric Marciano get it some time ago in his plugin FSUIPC Exporter. For only 45€ you may connect your cockpit to Wilco Airbus via FSUIPC. OAC will bring it to you open sourced and for free. 

Further details? Well, Wilco DLL files export some functions that expose some data to be consumed outside their libraries. By reading the machine code of the library, you can make some assumptions regarding the signature of these functions and the nature and purpose of each data field of the objects they return. From that point, it's possible to write a custom plugin that connects to FSX via SimConnect and loads the DLL accessing to these data. With some patient, you may log the values that are absolutely unknown and run the simulator to analyze their values as you interact the cockpit controls. 

So far, I am able to read the most basic parameters of the cockpit, including IRS status, configured transition altitude, active TCAS mode, barometer mode and throttle position among others. I identified as well how to read more interesting stuff, as the position of most switches and the values loaded into FCU. I hope to have most of the data available for reading using this approach. For writing the values, the thing isn't so clear. Probably determining the memory locations of the variables and writing the desired values there doesn't work. There are exported functions for sending commands in the DLL, but reversing their signature and their inputs could be a very hard task. An alternative approach could be to handle the writing operations by using FSUIPC macros. Anyway, too soon to discuss about that. 

The positive part of all this is that I am obtaining successful results. That's encouraging. The negative part is that reading machine code is an exhausting task. Most contemporaneous programmers are not familiarized with machine code and assembler, so it isn't easy to recognize high-level language forms and idioms in the machine code. Despite of that, it's funny to learn this kind of very-low-level programming skills, as when you realize that that weird subroutine being called is actually malloc() or sprintf() by only seeing the calling convention and the parameters pushed into stack. 

Well, these days I'll continue working on this reverse engineering with the goal of integrating it with OAC server. I mean to do that in a way that the component that extracts the data from Wilco is decoupled from the rest of OAC, so it may be used by other projects as an alternative to FSUIPC Exporter. In parallel, I will dedicate some time to the piece and structure models. I'll try to post more frequently to keep you updated. 

Happy flying!

