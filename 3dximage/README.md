3dximage
========

3dximage uses lib3dxdisp to display arbitrary images on the screen of the
3Dconnexion SpacePilot 6dof input device.

For best results prepare a black and white 240x64 pixel image, in an image
processing program, and tweak it until it looks good, then pass it to 3dximage
to send to the device. Alternatively you can let 3dximage itsel handle cropping,
rescaling, dithering and monochrome conversion, with some limited command-line
option for tweaking the result.

Build
-----
Before attempting to build 3dximage, you need to install libimago (at least
v2.3): https://github.com/jtsiomb/libimago

Then simply type make to build.

Usage examples
--------------
Send pre-made 240x64 b/w image to the device:

    ./3dximage foo.png

Choose 240x64 area from 100,92 to 340,156 from a larger image to display, also
invert the colors:

    ./3dximage -invert -offset 100,92 foo.png

Scale down a 640x170 region from a larger image:

    ./3dximage -geometry 640x170+100+92 foo.png

For more options run `./3dximage -help`.
