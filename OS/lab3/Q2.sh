#!/bin/bash
read -p "enter the path to directory : " path
cd "$path" 
read -p "enter the regex : " regex
if [ -z "$(find . -maxdepth 1 -type f -name "*$regex*")" ]; then
    echo "No matching files found."
    exit 1;
fi
printf "matchs found : \n"
find . -maxdepth 1 -type f -name "*$regex*"
