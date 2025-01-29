#!/bin/bash
customSort() {
    args=("$@") 
    n=$# 

    for ((i = 0; i < n; i++)); do
        for ((j = 0; j < n - 1 - i; j++)); do
            if [[ "${args[j]}" > "${args[$((j + 1))]}" ]]; then
                temp="${args[j]}"
                args[j]="${args[$((j + 1))]}"
                args[$((j + 1))]="$temp"
            fi
        done
    done
    echo "${args[@]}"
}
customSort "$@"
