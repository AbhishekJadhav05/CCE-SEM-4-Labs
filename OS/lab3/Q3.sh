#!/bin/bash
find . -type f -name "*.txt" | while read file; do
mv "$file" "${file%.txt}.text"
done
