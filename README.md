# backlight
Backlight control utility for Intel backlights in Linux
## Usage
To increase the luminance run the following with the desired increment.
```
backlight i {increment}
```
To decrease the luminance run the following with the desired decrement.
```
backlight d {decrement}
```
## Udev rules and user groups
To enable the use of `backlight` as a non-root user you will need some configuration.
First add the desired user to the `video` group.
```
usermod -a -G video {user}
```
Then add some new udev rules for the backlight at `/etc/udev/rules.d/backlight.rules`.
```
RUN+="/bin/chgrp video /sys/class/backlight/intel_backlight/brightness"
RUN+="/bin/chmod g+w /sys/class/backlight/intel_backlight/brightness"
```
