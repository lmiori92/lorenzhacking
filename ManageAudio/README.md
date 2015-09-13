ManageAudio : Audio Source Switching and Visualizer
===================================================

This project is about a device that is able to switch
between different audio sources, i.e. from different
music receivers and players.
As a side feature, the product is able to display
and visualize the current audio signal that passes
throug via a dedicated VU-meter and/or FFT mode.

Features:
    * Quick and full-featured GUI
    * Smart Switching. Smooth audio transitions.
    * Customizable and accurate audio visualizations

Audio switching
===============

The audio signal switching is accomplished by using
3 SPDT relays that are connected in a tree-like fashion.

The first relay starting from the bottom of the "tree"
switches 2 sources while its sibling another 2 sources.
The 3rd relay on the top does the switching between the
two siblings.

If the system requires additional audio sources it is still
possible to add a 4th relay, either on top or at the bottom
of the tree, starting a new leaf node.

Visualization
=============

The 2 stereo channels are read using the ADC subsystem of the
microcontroller. The sampling frequency is around 19khz, which
is more than sufficient for signal visualization porpuses.

Two modes are available: FFT and VU-meter; both have a couple
of appearance settings to match users' taste.

License
=======

For code licensing, please have a look at the source codes.

Version and Change log
======================

Version 0.1
    Initial Version
