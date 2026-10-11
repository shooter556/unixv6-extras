#!/usr/bin/env bash
set -euo pipefail

./uxtp/uxtp.sh -r -v -s -o 0:0 ${HOME}/vm/simhsys/unix/v6/extra.tap aredit fsck halt man setclock sh sysconf time trees tty
