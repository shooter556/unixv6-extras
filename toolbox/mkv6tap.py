#!/usr/bin/env python3
"""
mkv6tap.py - Build a single-file, self-bootstrapping SIMH .tap image from folders.

The tape trick (as used by the original 2BSD distribution tape): the tar
archive starts at tape block 0, and its FIRST member is the tar binary
itself.  Block 0 is that member's header, so the binary's data occupies
blocks 1..N.  Therefore a stock V6 system without tar can do:

    dd if=/dev/mt0 bs=1b skip=1 count=N of=tar    # pull tar out raw
    chmod 755 tar
    ./tar x                                       # extract everything

This script builds a V7-format tar archive from the given folders (each
member is stored under its folder's name, e.g. "tc/c00.c", so several
folders extract into separate subdirectories; owner 0/0, directory entries
omitted since old tar creates parent dirs itself), puts "tar" first, then
writes every 512-byte block as one tape record.

The tar binary written first is always the file "tar" next to this script
(a V6-runnable tar, e.g. the one from 2BSD).
Unlike tp (max 496 entries), this has no limit on the number of files.

Usage: mkv6tap.py DIR [DIR ...] dist.tap [-b N]
"""
import argparse
import os
import struct
import sys

BLOCK = 512


def v7_header(name, mode, size, mtime):
    """Build a V7-style tar header (linkflag NUL = regular file)."""
    hdr = bytearray(BLOCK)
    hdr[0:len(name)] = name.encode("latin-1")
    hdr[100:108] = b"%6o \0" % mode
    hdr[108:116] = b"%6o \0" % 0
    hdr[116:124] = b"%6o \0" % 0
    hdr[124:136] = b"%11o " % size
    hdr[136:148] = b"%11o " % mtime
    hdr[148:156] = b" " * 8
    hdr[148:156] = b"%6o\0 " % sum(hdr)
    return bytes(hdr)


def collect(top):
    """Yield (name, path) for every regular file under top, sorted.

    name keeps top's own folder name as its first component."""
    base = os.path.dirname(os.path.abspath(top))
    for root, dirs, files in os.walk(top):
        dirs.sort()
        for f in sorted(files):
            path = os.path.join(root, f)
            if os.path.isfile(path):
                yield os.path.relpath(os.path.abspath(path), base).replace(os.sep, "/"), path


def member(name, path, data=None):
    """Header plus block-padded data for one regular file."""
    if len(name) > 99:
        sys.exit(f"name too long for V7 tar (max 99): {name!r}")
    st = os.stat(path)
    if data is None:
        # some distribution files are mode 0; read them anyway,
        # the tape keeps the original mode
        if not os.access(path, os.R_OK):
            os.chmod(path, st.st_mode | 0o400)
        try:
            with open(path, "rb") as f:
                data = f.read()
        finally:
            os.chmod(path, st.st_mode)
    hdr = v7_header(name, st.st_mode & 0o7777, len(data), int(st.st_mtime))
    return hdr + data + bytes(-len(data) % BLOCK)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[1])
    ap.add_argument("src", nargs="+", help="folder(s) to put on the tape, each as a subdirectory")
    ap.add_argument("dst", help="output SIMH .tap file")
    ap.add_argument("-b", "--blocking", type=int, default=20,
                    help="pad archive to a multiple of N blocks (default 20)")
    if len(sys.argv) == 1:
        ap.print_help()
        sys.exit(0)
    args = ap.parse_args()
    for d in args.src:
        if not os.path.isdir(d):
            sys.exit(f"not a folder: {d}")

    tar_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "tar")
    with open(tar_path, "rb") as f:
        tar_bin = f.read()
    members = [member("tar", tar_path, tar_bin)]   # tar binary goes first

    seen = {"tar": tar_path}
    for d in args.src:
        for name, path in collect(d):
            if name in seen:
                print(f"WARNING: {name!r} in both {seen[name]!r} and {path!r}; "
                      "the later one wins on extraction")
            seen[name] = path
            members.append(member(name, path))

    archive = b"".join(members) + bytes(2 * BLOCK)

    # Pad to a whole number of tar records (blocking factor x 512 bytes),
    # like real tar does.  V7-era tar (e.g. 2BSD's) reads 20 blocks at a time; a short
    # final chunk makes the read run into the tape mark -> "tape read error".
    rec_bytes = args.blocking * BLOCK
    archive += bytes(-len(archive) % rec_bytes)

    # Each 512-byte block becomes one tape record: <len><data><len>
    rec_len = struct.pack("<I", BLOCK)
    with open(args.dst, "wb") as out:
        for i in range(0, len(archive), BLOCK):
            out.write(rec_len + archive[i:i + BLOCK] + rec_len)
        out.write(b"\0\0\0\0" * 2)          # two tape marks = logical end

    tar_size = len(tar_bin)
    count = (tar_size + BLOCK - 1) // BLOCK
    print(f"members written : {len(members)}")
    print(f"tape records    : {len(archive) // BLOCK} x {BLOCK} bytes")
    print(f"tar binary      : {tar_size} bytes -> {count} blocks")
    print(f"V6 command      : dd if=/dev/mt0 bs=1b skip=1 count={count} of=tar")
    if count != 49:
        print("NOTE: count differs from 49 (the 2BSD tar); use the value above.")


if __name__ == "__main__":
    main()
