<img src="https://github.com/bdring/FluidNC/wiki/images/logos/FluidNC.svg" width="600">

## Why yet another fork

Unfortunately for the owners of VFDs that are able to get and set the frequency in RPM, in FluidNC the pure RPM data from gCode gets corrupted on the system level by dividing it by 0.6 and thus losing some precision (i.e. 10 000 / 0.6 = 16666.6666~) so the reverse conversion might be off in the last digit because of the rounding which in turn might lead to VDD's screen showing 999 instead of 1000 or the current speed never reaching the requested one.  Another side effect is that all the debug messages will be off by 1.66, so if you enable debug and set the speed to 1000 RPM - you'd likely see something like this:

MSG:DBG: Synced speed. Requested:1666 current:1665

Now this kind of behavior is absolutely unacceptable to me, so I had to turn it off in the main codebase, but this fork will only work with Mitsubishi VFD at the moment, because any other spindle driver expects the speed not in RPM but in .01 Hz.

