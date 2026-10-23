#!/usr/bin/env bash
set -euo pipefail

# mkv6tap.py takes folders, so unpack the tar ball into a folder named
# 2bsd; its files go on the tape as 2bsd/...
STAGE="$(mktemp -d)"
trap 'rm -rf "${STAGE}"' EXIT
mkdir "${STAGE}/2bsd"
tar xzf 2bsd/spencer_2bsd.tar.gz -C "${STAGE}/2bsd"

mkdir -p ~/vm/iso/unix/2bsd
toolbox/mkv6tap.py \
    "${STAGE}/2bsd" \
    ~/vm/iso/unix/2bsd/2bsd.tap
