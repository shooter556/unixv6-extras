#!/bin/sh
# Change this to where the uxtp directory is installed
# and install this in a bin as 'uxtp'
UXPATH=${PWD}
exec python3 $UXPATH/uxtp/uxtp "$@"
