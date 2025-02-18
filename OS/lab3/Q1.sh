#!/bin/bash

args=$1

if [ -f "$args" ]; then
    echo "It's a file!"
elif [ -d "$args" ]; then
    echo "It's a directory!"
else
    echo "Invalid"
fi

