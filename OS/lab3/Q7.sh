#!/bin/bash
read -p "Enter the filename: " filename
if [[ ! -f "$filename" ]]; then
    echo "Error: File '$filename' not found!"
    exit 1
fi

sed -i 'n;d' "$filename"

echo "Even-numbered lines removed from $filename."

