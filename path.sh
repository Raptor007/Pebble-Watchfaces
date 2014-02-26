#!/bin/sh

# Get the script name; this method works for both direct calling and sourcing.
if [ "$0" = "-bash" ]
then
	THIS="${BASH_ARGV[0]}"
else
	THIS="$0"
fi

# Get the path to this script.
HERE="$(cd $(dirname $THIS) >& /dev/null && pwd)"

# If we didn't choose a specific PebbleSDK by environment variable, find the latest.
if [ -z "$PEBBLE_SDK" ]
then
	PEBBLE_SDK="$(cd $HERE && ls -d -t1 PebbleSDK-* 2>/dev/null | head -n1)"
fi

# Get the full path to the PebbleSDK/bin directory.
BIN_PATH="$HERE/$PEBBLE_SDK/bin"

# Add this PebbleSDK/bin directory to the start of the path, if it's not already there.
if [ -z "$(echo $PATH | grep $BIN_PATH)" ]
then
	export PATH="$BIN_PATH:$PATH"
fi
