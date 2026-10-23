#!/usr/bin/env bash
set -euo pipefail

# mkv6tap.py takes folders, so unpack the tar ball into a folder named
# unsw3; its files go on the tape as unsw3/...
STAGE="$(mktemp -d)"
trap 'rm -rf "${STAGE}"' EXIT
mkdir "${STAGE}/unsw3"
tar xzf unsw3/unsw3.tar.gz -C "${STAGE}/unsw3"

mkdir -p ~/vm/iso/unix/v6
toolbox/mkv6tap.py \
    "${STAGE}/unsw3" \
    ~/vm/iso/unix/v6/unsw3.tap
