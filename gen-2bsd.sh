#!/usr/bin/env bash
set -euo pipefail

toolbox/mkv6tap.py \
    2bsd/spencer_2bsd.tar.gz \
    ~/vm/iso/unix/2bsd/2bsd.tap
