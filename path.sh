#!/bin/sh

# Get the script name; this method works for both direct calling and sourcing.
if [ "$0" = "-bash" ]
then
	THIS="$PWD/${BASH_ARGV[0]}"
else
	THIS="$0"
fi

# Get the path to this script and the path to the PebbleSDK/bin directory.
HERE="$(cd $(dirname $THIS) >& /dev/null; pwd)"
BIN_PATH="$HERE/PebbleSDK-2.0.1/bin"

# Only add the PebbleSDK/bin directory to the path if it's not already there.
if [ "$(echo $PATH | grep $BIN_PATH)" = "" ]
then
	export PATH="$PATH:$BIN_PATH"
fi
