#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Function to sort an array of strings (Bubble Sort)
void sort_strings(char *arr[], int n) {
    char *temp;
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (strcmp(arr[j], arr[j + 1]) > 0) {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s string1 string2 ...\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    }
    else if (pid == 0) {
        // Child Process - Sort and display sorted strings
        printf("Child Process (Sorted Strings):\n");
        sort_strings(argv + 1, argc - 1);
        for (int i = 1; i < argc; i++) {
            printf("%s\n", argv[i]);
        }
    }
    else {
        // Parent Process - Wait for child to finish
        wait(NULL);
        printf("\nParent Process (Unsorted Strings):\n");
        for (int i = 1; i < argc; i++) {
            printf("%s\n", argv[i]);
        }
    }

    return 0;
}

