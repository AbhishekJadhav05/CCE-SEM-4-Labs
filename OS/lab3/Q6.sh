#!/bin/bash
for file in *; do
    if [[ -f "$file" ]]; then
        sed -i 's/^ex:/Example:/g; s/\.*ex:/\. Example:/g' "$file"
    fi
done

