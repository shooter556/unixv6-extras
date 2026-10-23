#!/usr/bin/env python3
"""
unar11.py - list or extract PDP-11 Unix ar archives on a modern host.

Supported formats:
  0177545  "new" ar (V7, 1BSD cont.a, PWB): 26-byte member header
  0177555  "old" ar (V6 and earlier):       16-byte member header

Usage:
  unar11.py -t cont.a              # list members
  unar11.py -x cont.a [-C outdir]  # extract members (default: current dir)
"""
import argparse
import os
import struct
import sys
import time

MAGIC_NEW = 0o177545
MAGIC_OLD = 0o177555


def le16(b, off):
    """Read a little-endian 16-bit word."""
    return struct.unpack_from("<H", b, off)[0]


def pdp_long(b, off):
    """Read a PDP-11 32-bit long: high word first, each word little-endian."""
    return (le16(b, off) << 16) | le16(b, off + 2)


def cstr(raw):
    """Decode a NUL-padded name field."""
    return raw.split(b"\0", 1)[0].decode("ascii", "replace")


def members(data):
    """Yield (name, date, uid, gid, mode, payload) for each member."""
    if len(data) < 2:
        sys.exit("file too short")
    magic = le16(data, 0)
    pos = 2

    if magic == MAGIC_NEW:
        # char name[14]; long date; char uid; char gid; int mode; long size;
        hsize = 26
        while pos + hsize <= len(data):
            name = cstr(data[pos:pos + 14])
            date = pdp_long(data, pos + 14)
            uid = data[pos + 18]
            gid = data[pos + 19]
            mode = le16(data, pos + 20)
            size = pdp_long(data, pos + 22)
            pos += hsize
            yield name, date, uid, gid, mode, data[pos:pos + size]
            pos += size + (size & 1)          # members are padded to even size

    elif magic == MAGIC_OLD:
        # char name[8]; int date[2]; char uid; char mode; int size;
        hsize = 16
        while pos + hsize <= len(data):
            name = cstr(data[pos:pos + 8])
            date = pdp_long(data, pos + 8)
            uid = data[pos + 12]
            mode = data[pos + 13]
            size = le16(data, pos + 14)
            pos += hsize
            yield name, date, uid, 0, mode, data[pos:pos + size]
            pos += size + (size & 1)

    else:
        sys.exit(f"unknown magic {magic:06o} (expected 177545 or 177555)")


def main():
    ap = argparse.ArgumentParser(description="PDP-11 Unix ar extractor")
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("-t", action="store_true", help="list members")
    g.add_argument("-x", action="store_true", help="extract members")
    ap.add_argument("-C", default=".", help="output directory for -x")
    ap.add_argument("archive")
    args = ap.parse_args()

    with open(args.archive, "rb") as f:
        data = f.read()

    if args.x:
        os.makedirs(args.C, exist_ok=True)

    for name, date, uid, gid, mode, payload in members(data):
        stamp = time.strftime("%Y-%m-%d %H:%M", time.gmtime(date))
        print(f"{mode & 0o7777:04o} {uid:3d}/{gid:<3d} {len(payload):7d} {stamp}  {name}")
        if args.x:
            # Keep only the base name so a bad header cannot escape outdir
            out = os.path.join(args.C, os.path.basename(name) or "noname")
            with open(out, "wb") as f:
                f.write(payload)
            os.chmod(out, (mode & 0o777) | 0o600)   # keep it readable/writable on the host
            os.utime(out, (date, date))


if __name__ == "__main__":
    main()
