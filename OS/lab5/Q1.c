#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    pid_t pid;
    pid = fork();

    if (pid < 0) {
        // Fork failed
        printf("Fork failed!\n");
        return 1;
    }
    else if (pid == 0) {
        // Child process
        printf("Child Process:\n");
        printf("  PID  : %d\n", getpid());   // Child's PID
        printf("  PPID : %d\n", getppid());  // Parent's PID
    }
    else {
        // Parent process
        printf("Parent Process:\n");
        printf("  PID  : %d\n", getpid());   // Parent's PID
        printf("  Child PID : %d\n", pid);  // Child's PID
    }

    return 0;
}

