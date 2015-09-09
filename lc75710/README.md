LC75710/LC75711/LC75712/LC75713 VFD Display Driver IC Library
=============================================================

It is common to find this sort of ICs in old electronics, as I did!

Moreover, VDF displays are pretty nice so why not writing down a simple
and bare library to handle the communication and the low-level commands.

Getting started - the result!
=============================

The library is capable of driving a VFD display like shown below.


![datasheet](doc/images/lc75710_hacked_display.jpg?raw=true "lc75710_hacked_display.jpg")
![datasheet](doc/images/lc75710_fft_display.avi?raw=true "lc75710_fft_display.avi")

Documentation - library
=======================

A doxygen-generated in-code documentation has been generated for a deep
understanding of the library. The HTML (index.html) is available under
the "doc" folder.

Documentation - datasheet
=========================

For your convenience, the doc folder holds a copy of the datasheet of the
LC757(10/11/12/13)

![datasheet](doc/datasheet/lc75710ne.pdf?raw=true "LC75710 Datasheet")

Installation
============

The library is installed as usual by copying its contents in the
$HOME/Arduino/libraries folder.
If you are lazy enough and you trust my install.sh Bash script, then:

$ ./install

is enough to install it.

Version
=======

1.0 - initial release

License
=======

GPLv3 - please have a look at source codes for the details and the license agreement

(C) 2015 Lorenzo Miori "IlLorenz"
