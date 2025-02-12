#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_STR_LEN 100

void swap(char *str1, char *str2) {
    char temp[MAX_STR_LEN];
    strcpy(temp, str1);
    strcpy(str1, str2);
    strcpy(str2, temp);
}

// Bubble Sort function
void bubble_sort(char arr[][MAX_STR_LEN], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (strcmp(arr[j], arr[j + 1]) > 0) {
                swap(arr[j], arr[j + 1]);
            }
        }
    }
}

// Selection Sort function
void selection_sort(char arr[][MAX_STR_LEN], int n) {
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            if (strcmp(arr[j], arr[min_idx]) < 0) {
                min_idx = j;
            }
        }
        swap(arr[i], arr[min_idx]);
    }
}

int main() {
    int n;
    printf("Enter number of strings: ");
    scanf("%d", &n);

    char strings[n][MAX_STR_LEN];

    printf("Enter %d strings:\n", n);
    for (int i = 0; i < n; i++) {
        scanf("%s", strings[i]);
    }

    pid_t pid1, pid2;

    //child got forked once
    pid1 = fork();

    if (pid1 == 0) {  // Child 1 (Bubble Sort)
        printf("\nChild 1 (Bubble Sort) - PID: %d\n", getpid());
        bubble_sort(strings, n);
        printf("Sorted strings using Bubble Sort:\n");
        for (int i = 0; i < n; i++) {
            printf("%s\n", strings[i]);
        }
        exit(0);
    }

    // Child got forked again :(
    pid2 = fork();

    if (pid2 == 0) {  // Child 2 (Selection Sort)
        printf("\nChild 2 (Selection Sort) - PID: %d\n", getpid());
        selection_sort(strings, n);
        printf("Sorted strings using Selection Sort:\n");
        for (int i = 0; i < n; i++) {
            printf("%s\n", strings[i]);
        }
        exit(0);
    }

    // Parent waits for any one child to terminate
    int status;
    pid_t finished_pid = wait(&status);

    printf("\nParent (PID: %d) detected termination of child process (PID: %d)\n", getpid(), finished_pid);
    printf("Unsorted strings (from parent process):\n");
    for (int i = 0; i < n; i++) {
        printf("%s\n", strings[i]);
    }

    return 0;
}

