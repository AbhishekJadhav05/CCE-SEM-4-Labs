#!/bin/bash
read -p "Enter file extension (including dot, e.g., .txt): " ext
read -p "Enter destination folder name: " dest
find . -maxdepth 1 -type f -name "*$ext" | while read file; do
    mv "$file" "$dest"
done
echo "operation completed"
