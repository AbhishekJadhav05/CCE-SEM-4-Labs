#!/bin/bash
filepath=$1
opts="${@:2}"

if [ ! -f "$filepath" ]; then
    echo "Error: The file '$file' does not exist."
    exit 1
fi

for opt in $opts
do
	case "$opt" in 
		-linecount)
			echo "Line count : "$(wc -l "$filepath")"";;
		-wordcount)
			echo "Word count : "$(wc -w "$filepath")"";;
		-charcount)
			echo "Word count : "$(wc -c "$filepath")"";;
		*)
			echo "meow";;
	esac
done
