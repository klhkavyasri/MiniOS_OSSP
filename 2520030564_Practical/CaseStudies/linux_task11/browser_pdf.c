#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    printf("Browser process started. PID: %d\n", getpid());

    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        printf("Child process created.\n");
        printf("Child PID: %d\n", getpid());
        printf("Opening PDF viewer using exec...\n");

        execlp("xdg-open", "xdg-open", "sample.pdf", (char *)NULL);

        perror("exec failed");
        return 1;
    }

    printf("Parent browser process continues.\n");
    printf("Parent PID: %d\n", getpid());

    wait(NULL);

    printf("PDF viewer process finished.\n");
    printf("Browser process exiting.\n");

    return 0;
}
