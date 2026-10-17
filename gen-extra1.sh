#!/usr/bin/env bash
set -euo pipefail

./uxtp/uxtp.sh -r -v -s -o 0:0 ${HOME}/vm/simhsys/unix/v6/extra1.tap \
aredit em fsck halt man setclock sh sysconf time trees tty
