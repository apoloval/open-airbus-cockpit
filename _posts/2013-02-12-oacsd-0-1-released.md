---
layout: post
title: "Open Airbus Cockpit Software Distribution v0.1 Beta is released"
category: Software
tags: [oacsd]
---

I should post more than I do, but unfortunately the spare time I have to work on OAC is limited, and I'd rather reserve it to code. But let me break the silence to announce a very important milestone: Open Airbus Cockpit Software Distribution (OACSD) version 0.1 has been released. 

I think I've never talked about OACSD, so I guess some words would be appreciated. OACSD is the name given to the software distribution of OAC Project. From now on, any software piece will be packed in a installer and distributed under that product name. 

What software? Well, if you follow the project progress, you might have some clues. The best way to figure out is to check out the release notes for v0.1 but let me summarize.

* The first version of Wilco Exporter. Yes, finally we did it. Reverse engineering bore fruit, so you can enjoy a open-source, free-of-charge replacement of FSUIPC Exporter. In this version, it have some constraints (only EFIS Control Panel and FCU are supported, and only for A320 CFM) that will be eliminated in further releases. 

* liboac-commons library. This is some utility code used by Wilco Exporter that will be reused in other components in the near future. If you are a developer and are interested in C++ code that makes it easier to work with FSUIPC and SimConnect, you should check it out. 
If you are interested in OACSD after this brief summary, you may download it following this link. 

If you have any question or comment, please do not hesitate to ask with a comment, email, bug report, etc. 

Happy flying!
