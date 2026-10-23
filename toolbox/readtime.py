#!/usr/bin/env python3
import sys
import datetime

# Usage: v6time.py <disk-image> [start-block]
# start-block: filesystem start in 512-byte blocks (for RP03 partitions), default 0
if len(sys.argv) < 2:
    sys.exit(f"usage: {sys.argv[0]} <disk-image> [start-block]")

image = sys.argv[1]
start = int(sys.argv[2]) if len(sys.argv) > 2 else 0

with open(image, "rb") as f:
    # Superblock is block 1 of the filesystem, s_time is at offset 412 in it
    f.seek((start + 1) * 512 + 412)
    b = f.read(4)

# PDP-11 32-bit layout: high word first, each word little-endian
t = (int.from_bytes(b[0:2], "little") << 16) | int.from_bytes(b[2:4], "little")
print(t, datetime.datetime.fromtimestamp(t, datetime.timezone.utc))
