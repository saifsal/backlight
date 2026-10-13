# backlight

`backlight` sets the brightness of an Intel GPU panel on Linux. The kernel publishes the current level and the hardware maximum as files under `/sys/class/backlight/intel_backlight/`. This program reads those files, raises, lowers, or sets the level as a percentage of the maximum, clamps the result, and writes it back. It can also print the current level as a fraction of the maximum. A shell script or a window-manager key binding can call it on each key press.

## Features

- Raise (`i`), lower (`d`), or set (`s`) brightness as a percentage of `max_brightness`.
- Print the current level (`g`) as a fraction of the maximum.
- Clamp every update to the range the hardware reports, from `0` through `max_brightness`.
- One C source file and the C standard library. No configuration file and no background service.

## Examples

Starting from `max_brightness` of `1000` and `brightness` of `200`:

```
$ backlight i
$ backlight i 10
$ backlight d 5
$ backlight s 50
$ backlight g
Backlight: 0.50
```

The stored brightness after each command is `210`, then `310`, then `260`, then `500`. `g` prints the fraction and writes `500` back. `i`, `d`, and `s` are silent on success.

`backlight s 0` stores `0`. `backlight s 100` stores the maximum.

## Prerequisites

- Linux with these Intel backlight files:
  - `/sys/class/backlight/intel_backlight/brightness`
  - `/sys/class/backlight/intel_backlight/max_brightness`
- A C compiler and GNU make, to build from source.
- Permission to read both files and to write `brightness`. Root has that permission when the files exist.

The paths are compiled into the program. Other backlight names, including `acpi_video0` and AMD or NVIDIA devices, are left untouched.

## Install

```
git clone https://github.com/saifsal/backlight.git
cd backlight
make
sudo make PREFIX=/usr/local install
```

`make` builds `./backlight` in the repository root. The install line copies it to `/usr/local/bin/backlight` and sets mode `755`.

Set `PREFIX` on the `make install` command. The Makefile leaves `PREFIX` empty, and an empty `PREFIX` installs to `/bin/backlight`.

A staged install, useful for packaging, takes `DESTDIR` as well:

```
make PREFIX=/usr/local DESTDIR="$PWD/stage" install
```

Remove an install with the same prefix (and the same `DESTDIR`, if you used one):

```
sudo make PREFIX=/usr/local uninstall
```

## Usage

```
backlight <i|d|s|g> [percentage]
```

| Command | Effect |
| --- | --- |
| `i` | Add `percentage` percent of the maximum. |
| `d` | Subtract `percentage` percent of the maximum. |
| `s` | Set brightness to `percentage` percent of the maximum. |
| `g` | Print `Backlight: X.XX`, then write the current brightness. |

`percentage` is optional and defaults to `1`. It is an integer from 0 to 100. The change in hardware units is

```
max_brightness * percentage / 100
```

using integer division, so the stored value is a whole number.

The program takes one command and at most one percentage. With no arguments, or with extra arguments, it prints a usage line to stderr and exits 1 without opening sysfs. The line is `Usage: `, the name used to invoke the program, then ` <i|d|s|g> [percentage]`. Invoked as `backlight`, that is:

```
Usage: backlight <i|d|s|g> [percentage]
```

Exit status is `0` on success. Argument errors, unreadable sysfs files, a sysfs value that is not a non-negative integer, and a rejected percentage exit `1`. A failed write of `brightness` exits `255`.

`g` always continues into that write. It prints the ratio `brightness / max_brightness` with two decimal places (`Backlight: 0.50` when the level is half of the maximum), then stores the clamped current value. A valid percentage on `g` does not change the printed value: `backlight g 40` still prints the current fraction. `backlight g 101` exits 1 before the print and does not write.

Only the first character of the command is read, and the comparison is case-sensitive. `backlight increase` is `i`. `backlight I` is rejected with `Invalid command: I`.

On a machine without the Intel backlight files, a command fails on stderr with `No such file or directory` for each missing path and exits 1.

## Configuration

Nothing is read from a configuration file, the environment, or flags beyond the command and the percentage.

Root needs no further setup when the sysfs files exist.

A non-root user must be able to write `brightness` (and read both files). This repository's setup for that is the `video` group, plus udev rules that hand the Intel brightness file to that group.

As root, add the account to the group:

```
usermod -a -G video {user}
```

Log in again so the new group is on the session.

Create `/etc/udev/rules.d/backlight.rules`:

```
ACTION=="add", SUBSYSTEM=="backlight", KERNEL=="intel_backlight", RUN+="/bin/chgrp video /sys/class/backlight/intel_backlight/brightness"
ACTION=="add", SUBSYSTEM=="backlight", KERNEL=="intel_backlight", RUN+="/bin/chmod g+w /sys/class/backlight/intel_backlight/brightness"
```

Those two `RUN` actions are the ones this project has documented. The `ACTION`, `SUBSYSTEM`, and `KERNEL` matches limit them to the `intel_backlight` device. The earlier README listed the `RUN` lines alone; a udev rule with no match keys runs for every device event.

Reload the rules and apply them to a backlight device that is already present:

```
udevadm control --reload-rules
udevadm trigger --subsystem-match=backlight
```

Run those as root. Brightness permissions are recreated when the device appears, which is why the rule is installed rather than a one-off `chmod`.

## Development and testing

```
make
make CFLAGS="-Wall -Wextra -pedantic -std=c99"
make clean
```

`make` is the build. With `CFLAGS="-Wall -Wextra -pedantic -std=c99"` the source compiles and emits no warnings. `make clean` deletes `backlight` and `backlight.o`.

`CC`, `CFLAGS`, and `LDFLAGS` select the compiler and flags. For example, `make CC=gcc`.

There is no test suite. `make test` is not a target and exits 2. Exercising the commands requires the Intel sysfs files, or a private build whose path macros point somewhere else. Leave `backlight`, `backlight.o`, and any such private copy out of commits.

Contributor instructions for agents are in `AGENTS.md`.

## Status

The published history is a single commit, message `0.0.1`, from 24 July 2021. There are no release tags and no roadmap in the repository. The program implements the four commands above for the Intel backlight sysfs interface.

A few limits are visible in the current source:

- The read buffer holds 7 characters. A value of 8 digits or more is truncated, and the following write can store that shorter number. `g` does this too, because `g` writes.
- A percentage that rounds to 0 hardware units, such as 1 percent of a maximum below 100, still rewrites the file.
- `backlight s` with no percentage sets the level to 1 percent of the maximum.
- Written values have no trailing newline.

## Licence

GNU General Public License, version 3. The licence text is [`LICENSE`](LICENSE).
