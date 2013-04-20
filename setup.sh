#!/bin/sh

if [ -z "$1" ]; then
	echo "Usage: $0 <watchface>"
else
	../tools/create_pebble_project.py --symlink-only ../sdk/ "$1"
fi
