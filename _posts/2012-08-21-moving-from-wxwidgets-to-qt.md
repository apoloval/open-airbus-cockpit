---
layout: post
title: "Moving from wxWidgets to QT"
category: Software
tags: [wxwidgerts, qt, c++]
---

The server side of OAC is implemented in C++ language. This is due to two main reasons.
Flight simulators, as almost any real-time application that must fit some  responsiveness requirements, is implemented in a compiled language, typically C or C++. Using Java for this kind of software is like driving in F1 in a truck. And there is a chance that OAC requires such a grade of integration with the simulator that requires writing plugins for it, of course, in C/C++.
C++ is my favorite language.

The very first implementation of the server was just a set of C++ classes that wraps serial device handling behavior and some UI based on wxWidgets for testing the device. Why wxWidgets? It seemed to be a good idea. C++, open source, portable, etc. But the nightmare comes into scene when you plan to work on C++11.

<!--more-->

C++11 is the (new?) revision of the language appeared in (you guess?) 2011. It includes new fascinating features that makes C++ more productive and easy to use. The cons is that — well, the new features are pretty disruptive, and not easy to implement for compiler developers. In the middle 2012, not all new features are present in all compilers. But some of these features worth the risk of early-adopting the latest versions of C++ compilers. 

wxWidgets does not work when using a C++11 compiler. Why? Well, I don't know the details, but there is a reported bug in the project that claims not to compiler with -std=c++11 flag. And I had the same issue compiling OAC. 

The lack of support of C++11 could be enough by its own to discard wxWidgets. But there are more reasons. wxWidgets is old, very old. It was designed when C++ was pretty young, and some of the most basic features we expect from a C++ compiler wasn't invented yet. More concisely, the way it have to connect widget events and handlers (signals and slots in QT terminology) is weird. It is based on pointers to function members with the restriction that receiver object must be the window where the event was generated. That means that your handlers must be function members of your window class. High coupling. Not funny. In addition, the look&feel of a wxWidgets application is not fully integrated with the target desktop. It was very frustrating to see that group boxes are framed by a high-contrasted white line rather than typical Windows group boxes with a fine grey line. 

In summary, wxWidgets has been discarded. I am rewriting the test utility using the QT framework, which not only supports C++11 but have in its roadmap to integrate some of its features (e.g., lambda functions) in its source code. I was unwilling to use it due to my tendency to believe that big code monsters are not good solutions (QT SDK occupies over 2GB on disk). But, after the typical issues that appear when you start with a new IDE and building tools, we are moving forwards. You may find in the repository the utility tool, capable of launching the main window and connecting to the device by requested COM port. Last night I worked on the FCU test window, which I estimate to have it coded by the end this week. 

After this little break of code refactoring, I will come back to the breadboard. Next step is to connect the inputs (rotary encoders and buttons) to the Arduino board and integrate them with the test utility. That will be the last step of the device development. Next chapter will be a little harder: integrate the server with FSX and Wilco Airbus addon. 
