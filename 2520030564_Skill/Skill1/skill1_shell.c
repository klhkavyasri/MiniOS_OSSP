#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#define MAX_INPUT 100

struct termios original;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &original);

    struct termios raw = original;
    raw.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    atexit(disableRawMode);
}

void processCommand(char *command) {
    if (strcmp(command, "hello") == 0) {
        printf("\nHello Kavya! Shell is working.\n");
    }
    else if (strcmp(command, "help") == 0) {
        printf("\nCommands: hello, help, exit\n");
    }
    else if (strlen(command) > 0) {
        printf("\nUnknown command: %s\n", command);
    }
}

int main() {
    char input[MAX_INPUT];
    int length;

    enableRawMode();

    while (1) {
        printf("\nmyshell> ");
        fflush(stdout);

        length = 0;
        input[0] = '\0';

        while (1) {
            char c;

            read(STDIN_FILENO, &c, 1);

            if (c == '\n' || c == '\r') {
                input[length] = '\0';

                if (strcmp(input, "exit") == 0) {
                    printf("\nExiting shell...\n");
                    disableRawMode();
                    return 0;
                }

                processCommand(input);
                break;
            }

            else if (c == 127 || c == 8) {
                if (length > 0) {
                    length--;
                    input[length] = '\0';
                    printf("\b \b");
                    fflush(stdout);
                }
            }

            else if (c >= 32 && c <= 126) {
                if (length < MAX_INPUT - 1) {
                    input[length++] = c;
                    input[length] = '\0';

                    putchar(c);
                    fflush(stdout);
                }
            }
        }
    }

    return 0;
}
