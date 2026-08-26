#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#define MAX_HISTORY 50
#define MAX_INPUT 100

char history[MAX_HISTORY][MAX_INPUT];
int history_count = 0;
int history_pos = 0;

struct termios original_terminal;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_terminal);
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &original_terminal);

    struct termios raw = original_terminal;

    raw.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    atexit(disableRawMode);
}

void addHistory(const char *command) {
    if (strlen(command) == 0)
        return;

    if (history_count < MAX_HISTORY) {
        strcpy(history[history_count], command);
        history_count++;
    } else {
        for (int i = 1; i < MAX_HISTORY; i++)
            strcpy(history[i - 1], history[i]);

        strcpy(history[MAX_HISTORY - 1], command);
    }

    history_pos = history_count;
}

void showHistory() {
    printf("\n\nCommand History:\n");

    for (int i = 0; i < history_count; i++)
        printf("%d  %s\n", i + 1, history[i]);
}

void clearLine(int length) {
    for (int i = 0; i < length; i++)
        printf("\b \b");

    fflush(stdout);
}

int main() {
    char input[MAX_INPUT];
    int length = 0;

    enableRawMode();

    while (1) {
        printf("\nminishell> ");
        fflush(stdout);

        length = 0;
        input[0] = '\0';
        history_pos = history_count;

        while (1) {
            char c;

            if (read(STDIN_FILENO, &c, 1) != 1)
                continue;

            /* Enter */
            if (c == '\n' || c == '\r') {
                input[length] = '\0';

                if (strcmp(input, "exit") == 0) {
                    printf("\nExiting shell...\n");
                    disableRawMode();
                    return 0;
                }

                if (strcmp(input, "history") == 0) {
                    showHistory();
                } else if (length > 0) {
                    printf("\nYou entered: %s\n", input);
                }

                addHistory(input);
                break;
            }

            /* Backspace */
            else if (c == 127 || c == 8) {
                if (length > 0) {
                    length--;
                    input[length] = '\0';
                    printf("\b \b");
                    fflush(stdout);
                }
            }

            /* Escape sequence */
            else if (c == 27) {
                char seq[2];

                if (read(STDIN_FILENO, &seq[0], 1) != 1)
                    continue;

                if (seq[0] != '[')
                    continue;

                if (read(STDIN_FILENO, &seq[1], 1) != 1)
                    continue;

                /* UP ARROW */
                if (seq[1] == 'A') {
                    if (history_pos > 0) {
                        clearLine(length);
                        history_pos--;

                        strcpy(input, history[history_pos]);
                        length = strlen(input);

                        printf("%s", input);
                        fflush(stdout);
                    }
                }

                /* DOWN ARROW */
                else if (seq[1] == 'B') {
                    clearLine(length);

                    if (history_pos < history_count - 1) {
                        history_pos++;
                        strcpy(input, history[history_pos]);
                        length = strlen(input);
                        printf("%s", input);
                    } else {
                        history_pos = history_count;
                        length = 0;
                        input[0] = '\0';
                    }

                    fflush(stdout);
                }
            }

            /* Normal character */
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
