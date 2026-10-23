#!/usr/bin/env bash
set -euo pipefail

./vanilla/uxtp/uxtp.sh -r -v -s -o 0:0 ${HOME}/vm/simhsys/unix/v6/1sd.tap \
1bsd
