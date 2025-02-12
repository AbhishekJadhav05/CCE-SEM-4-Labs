#!/bin/bash
if [[ $# -eq 0 ]]; then
    echo "Usage: $0 pattern1 [pattern2 ...] "
    exit 1
fi
read -p "Enter the filename: " filename
if [[ ! -f "$filename" ]]; then
    echo "Error: File '$filename' not found!"
    exit 1
fi

while true; do
    echo -e "\nMenu:"
    echo "1. Search for patterns in the file"
    echo "2. Delete all occurrences of patterns in the file"
    echo "3. Exit"
    read -p "Enter your choice: " choice

    case $choice in
        1)
            echo -e "\nLines containing the patterns:"
            grep -E "$*" "$filename"
            ;;
        2)
            echo -e "\nDeleting all occurrences of patterns..."
            sed -i -E "s/$*//g" "$filename"
            echo "Deletion completed!"
            ;;
        3)
            echo "Exiting..."
            exit 0
            ;;
        *)
            echo "Invalid choice! Please enter 1, 2, or 3."
            ;;
    esac
done
