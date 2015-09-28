#!/bin/bash
# syncbp.sh - bring config files up to date.

CONFIG="$HOME"/.config/gengo/
# Check that our config dir exists
if [[ ! -d "$CONFIG" ]]
then
	mkdir -p "$CONFIG"
fi

# list of files to check
for i in getoptionsBP.c  getoptionsBP.h  mainBP.c MakefileBP
do
	echo "$CONFIG"/"$i"
	# update target if source is newer or target does not exist.
	if [[ "$i" -nt "$CONFIG"/"$i" ]]
	then
		cp "$i" "$CONFIG"/
		echo Made a copy.
	fi
done
