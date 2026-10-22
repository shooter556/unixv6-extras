#!/usr/bin/env bash
set -euo pipefail

uxtp/mkv6tap.py \
    2bsd/spencer_2bsd.tar.gz \
    ~/vm/iso/unix/2bsd/2bsd.tap
