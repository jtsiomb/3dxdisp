3dxdisp - 3Dconnexion SpacePilot display control library
========================================================

![logo](http://nuclear.mutantstargoat.com/sw/misc/3dxdisp-512.jpg)

3dxdisp is a very simple C library for controlling the LCD on a 3Dconnexion
SpacePilot 6-dof input device. See `example/src/example.c`, and the `3dxdisp.h`
header file, for details on how to use it.

Eventually this code will be integrated in the [free spacenav](http://spacenav.sourceforge.net)
project, but it's also useful as a standalone library, to write interesting
programs that use the otherwise pointless LCD for desktop notifications,
time/date, system monitoring, etc.

Currently 3dxdisp only works on GNU/Linux systems. But since it relies on
hidapi, it should be easily portable to MacOS X and Windows, by just dropping
the necessary source files from the hidapi code, and tweaking the Makefile.

License
-------
Copyright (C) 2018 John Tsiombikas <nuclear@member.fsf.org>

This program is free software. Feel free to use, modify, and/or redistribute it
under the terms of the MIT/X11 license. See LICENSE for details.

Build
-----
The only dependency is libudev (required by the GNU/Linux implementation of
HIDAPI). After you have install libudev, just type make to build the library,
and then change into example/ and type make to build the example program.

To be able to run the example program, or your own 3dxdisp programs, as an
unpriviledged user, the space pilot hidraw device must have the appropriate
permissions. The best way to do that, is to add a new file `3dxdisp.rules` to
`/etc/udev/rules.d/` with the following contents:

    KERNEL=="hidraw[0-9]*", ATTRS{idVendor}=="046d", ATTRS{idProduct}=="c625", MODE="0666"
