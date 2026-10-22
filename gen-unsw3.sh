#!/usr/bin/env bash
set -euo pipefail

uxtp/mkv6tap.py \
    unsw3/unsw3.tar.gz \
    ~/vm/iso/unix/v6/unsw3.tap
