AGENTS.md is the canonical source for these instructions.

# backlight

## Overview

`backlight` is a Linux command-line utility that sets the panel brightness exposed by the Intel backlight driver. It reads the current level and the hardware maximum from sysfs, applies one of four commands, clamps the result, and writes the brightness file back. Callers are expected to be a shell, a script, or a key binding.

The only files it touches are:

- `/sys/class/backlight/intel_backlight/brightness`
- `/sys/class/backlight/intel_backlight/max_brightness`

The public history is one commit, message `0.0.1`, dated 24 July 2021. The binary does not print a version. There are no release tags, no issue tracker entries, no CI, no test suite, and no roadmap in the tree. The GitHub description is "Backlight control utility for Intel backlights in Linux".

## Repository layout

- `backlight.c` — the whole program.
- `Makefile` — `all`, `clean`, `install`, and `uninstall`.
- `LICENSE` — GNU General Public License, version 3.
- `README.md` — human-facing setup and usage.
- `AGENTS.md` — canonical agent instructions.
- `CLAUDE.md` — copy of these instructions, plus a one-line canonical-source header.
- `.github/copilot-instructions.md` — the same copy as `CLAUDE.md`.

The tree has no CI workflows, no `.gitignore`, and no test directory.

## Setup

Build requirements are GNU make and a C compiler on `PATH` as `cc`. Override the compiler with `CC`.

Runtime requirements are the two sysfs files above, permission to read both, and permission to write `brightness`. Root meets the permission requirement when the files exist. Another user needs write access to `brightness`. The human setup for that is the `video` group and the udev rules in `README.md`.

```
git clone https://github.com/saifsal/backlight.git
cd backlight
make
sudo make PREFIX=/usr/local install
```

`PREFIX` is unset in the Makefile. Pass it on the command line. A staged install also takes `DESTDIR`:

```
make PREFIX=/usr/local DESTDIR=/tmp/backlight-stage install
```

The machine used to check this repository has no `/sys/class/backlight` tree. The build still succeeds. Running a command here fails because those paths are missing.

## Build, run and test

These commands were run against this tree (GNU Make 4.3, system `cc` via Ubuntu Clang 18.1.3):

```
make
make CFLAGS="-Wall -Wextra -pedantic -std=c99"
make clean
make PREFIX=/usr/local DESTDIR=/tmp/backlight-stage install
make PREFIX=/usr/local DESTDIR=/tmp/backlight-stage uninstall
```

`make` compiles `backlight.c` to `backlight.o` and links `./backlight`. The `-Wall -Wextra -pedantic -std=c99` build emits no warnings. `make clean` removes `backlight` and `backlight.o`. The staged `install` creates `DESTDIR/PREFIX/bin/backlight` with mode `755`. `uninstall` removes that file. `install` depends on `all`; `uninstall` does not.

```
make test
```

This fails. Make exits 2 with "No rule to make target 'test'." There is no automated test suite.

With no backlight sysfs, the built binary does this:

```
./backlight
```

stderr: `Usage: ./backlight <i|d|s|g> [percentage]`. Exit 1. It does not open sysfs.

```
./backlight i
```

stderr, then exit 1:

```
/sys/class/backlight/intel_backlight/brightness: No such file or directory
/sys/class/backlight/intel_backlight/max_brightness: No such file or directory
```

Command behaviour was checked with a temporary copy whose path macros pointed at fixture files. That copy is not part of the repository. From a maximum of `1000` and a current brightness of `200`, this sequence ran:

```
backlight i       # stores 210
backlight i 10    # stores 310
backlight d 5     # stores 260
backlight s 50    # stores 500
backlight g       # prints "Backlight: 0.50" and stores 500
```

`i`, `d`, and `s` print nothing on success. The optional percentage defaults to `1`. The hardware step is `max_brightness * percentage / 100` in integer arithmetic.

Usage is one command and at most one percentage. Zero arguments, or more than two, print the usage line to stderr and exit 1 before any file is opened.

Exit status is `0` on success, `1` when argument, read, or percentage checks fail, and `255` when the final write fails (`main` returns `-1` from `write_file`).

Checks run in this order: argument count, both sysfs reads, percentage, command letter, clamp, write. `backlight x 101` therefore reports `Percentage must be an integer between 0 and 100` and does not reach `Invalid command`.

## Code style and conventions

- One C translation unit. It compiles as C99. Indentation is two spaces. Opening braces stay on the same line. A single-statement `if` in `main` has no braces.
- East `const` on parameters and locals (`char const *const`, `long const`).
- The sysfs paths and `BUFFER_SIZE` (`8`) are macros at the top of `backlight.c`.
- `read_file` and `read_percentage` return `-1` on failure. `write_file` returns `0` or `-1`. Messages go to stderr through `perror` or `fprintf`. `g` prints to stdout.
- The command is `argv[1][0]` only, compared as a character. `increase` is `i`. `I` is rejected.
- `backlight.c` has no comments and no licence banner. The Makefile header points at `LICENSE`.
- The Makefile sets `SRC` and `OBJ`, uses a `.c.o` suffix rule, and honours `CC`, `CFLAGS`, `LDFLAGS`, `PREFIX`, and `DESTDIR`. `CFLAGS` and `LDFLAGS` are empty unless the invocation sets them. `.PHONY` is `all`, `clean`, `install`, `uninstall`. Recipe lines are tabs.
- No formatter, linter, or editor config is checked in.

## CI and tooling

Not applicable for CI. The repository has no GitHub Actions workflows and no other CI configuration.

Local tooling is GNU make and the C compiler selected by `CC` (default `cc`). There is no separate lint or format step.

## Working rules for agents

- Keep changes small and focused.
- Run the tests before committing. There is no test suite, and `make test` is not a target. Run `make` before committing, then `make clean`, so `backlight` and `backlight.o` are not added.
- Never commit secrets.
- Update the docs when behaviour changes. That includes `README.md` and these instruction files.
- Use conventional commit messages.
- Keep `AGENTS.md`, `CLAUDE.md`, and `.github/copilot-instructions.md` in sync. `AGENTS.md` is the source of truth. The other two files must be the same text, apart from at most a one-line header noting that `AGENTS.md` is canonical. Do not use symlinks.

## Gotchas

- The optional number is an integer percentage of `max_brightness`, from 0 to 100. Hardware units are `max_brightness * percentage / 100`. The first README described that number as a raw increment. Omitting it means 1, so `backlight s` with no percentage sets the level to 1 percent of the maximum.
- `g` prints `Backlight: ` and the ratio `brightness / max_brightness` with two digits after the decimal (`0.50` for 500 of 1000). It then clamps and writes `brightness`. It is not a read-only command. A valid percentage passed to `g` is parsed and ignored. An invalid one fails first, so nothing is printed or written. A failed write still leaves the fraction on stdout and exits 255.
- Every successful command writes `brightness`, including a step that rounded to 0 and a `g` of an unchanged value. The format is `%ld` with no trailing newline.
- Integer division truncates the step. One percent of a maximum below 100 is 0. `backlight s 50` with a maximum of 937 stores 468.
- The result is clamped to `0` through `max_brightness` before the write, including `g`.
- `read_file` keeps 7 characters (`fgets` with `BUFFER_SIZE` 8). Seven-digit values still parse. Eight digits are truncated: a maximum of `10000000` and a brightness of `5000000` made `g` print `Backlight: 5.00` and store `1000000`.
- `read_file` accepts a numeric prefix (`100abc` is 100). `read_percentage` requires the whole argument to be consumed. `+10` and a leading space are accepted. A trailing space is rejected. Values below 0 or above 100 are rejected.
- A `max_brightness` of 0 makes the step 0. With both sysfs values at 0, `g` printed `Backlight: -nan`.
- Sysfs is read before the command letter is checked. A missing device reports `No such file or directory` for `backlight x` as well as for `backlight i`.
- `PREFIX` empty and `DESTDIR` empty make `make install` copy the binary to `/bin/backlight` (`mkdir -p /bin`). Always pass `PREFIX`.
- `make` leaves `backlight` and `backlight.o` in the work tree. There is no `.gitignore`. Do not commit them.
