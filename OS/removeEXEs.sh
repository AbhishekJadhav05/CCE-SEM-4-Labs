#!/bin/bash

# Find and remove all .exe files recursively from the current directory
find . -type f -name "*.exe" -exec rm -f {} \;

echo "All .exe files have been removed."

