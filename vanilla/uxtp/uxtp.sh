#!/bin/sh
# Change this to where the uxtp directory is installed
# and install this in a bin as 'uxtp'
UXPATH=$(dirname "$(readlink -f "$0")")
exec python3 $UXPATH/uxtp "$@"
