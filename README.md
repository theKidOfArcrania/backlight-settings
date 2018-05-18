# Backlight Settings

This is yet another program that adjust a laptop screen backlighting.

This works only for `intel_backlight` compatible settings. This probably
should've already worked with `xbacklight` but apparently, in my case, it
doesn't. 

This particular solution directly writes to the `intel_backlight/brightness`
file. Note that there is a udev rule to set this file as the `video` group, and
then we have a setgid binary as video group. 

This sets the backlight as a percentage of maximum brightness.
