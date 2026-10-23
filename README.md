# unixv6-extras

Tools, sources and notes for running and extending Unix Sixth Edition (V6)
on the [SIMH](https://github.com/simh/simh) PDP-11 simulator.

This is a fork of Peter Collinson's
[unixv6-extras](https://github.com/pcollinson/unixv6-extras). His work is
kept in [vanilla](vanilla), with a few bug fixes; everything else here was
added on top of it.

## What's here

* __[vanilla](vanilla)__ - Peter Collinson's original tools and notes: SIMH
  setup, the _uxtp_ tape program, _fsck_, kernel rebuild scripts, _halt_,
  the new tty driver, the new time code, _aredit_ and more. Start with its
  [README](vanilla/README.md).

* __[type_C](type_C)__ - builds the "typesetter" C toolchain on a stock V6
  system, from source only: the C compiler, the UNSW _cc_ driver and
  assembler, a C library with long arithmetic, _ar_ and _ld_ for the new
  archive format, and the standard I/O library. Each step is checked on
  the system itself. See [type_C/READ_ME](type_C/READ_ME).

* __[bsd1](bsd1)__ - builds the 1BSD software from its sources on a
  stock V6 system with the type_C toolchain, without the binaries
  on the tape: _ashell_ first, then the tools its make scripts use
  and _libX.a_, then the terminal data bases with _sethome_,
  _ttytype_ and _tset_, a _login_ and _su_ that keep them up to
  date, and the editor _ex_ 1.1 with visual mode on a vt100. See
  [bsd1/READ_ME](bsd1/READ_ME).

* __[toolbox](toolbox)__ - helper scripts that run on the host:
  * _mkv6tap.py_ - makes a SIMH tape image from folders. The first file
    on the tape is a _tar_ that runs on V6 (the file _tar_ here), so a
    system without _tar_ can unpack the tape.
  * _unar7.py_ - lists or extracts PDP-11 _ar_ archives, both the old V6
    format and the new one used by V7 and the 1BSD _cont.a_ files.
  * _readtime.py_ - prints the time stored in the superblock of a V6
    disk image.

* __[em](em)__ - the _em_ editor from Queen Mary College.

* __[1bsd](1bsd)__, __[2bsd](2bsd)__, __[unsw3](unsw3)__ - the 1BSD and
  2BSD distributions and the UNSW 3 tape, as tar balls.

* __gen-*.sh__ - make the tape images, run from the top of the repository:

  | script | tape |
  | --- | --- |
  | `gen-extra1.sh <distro>` | the tools in _vanilla_ and _em_, read with _tp_ |
  | `gen-1bsd.sh` | 1BSD, read with _tp_ |
  | `gen-2bsd.sh` | 2BSD, built with _mkv6tap.py_ |
  | `gen-unsw3.sh` | UNSW 3, built with _mkv6tap.py_ |

  On V6, a tape made by _mkv6tap.py_ is read with:

  ```sh
  dd if=/dev/mt0 of=tar bs=512 skip=1 count=49
  chmod 755 tar
  ./tar x
  ```

## Licenses

Several licenses apply to this repo - see [License](License.md).
