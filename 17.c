#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

#define ALERT_SOUND  "\07"  // Ctrl-G for alert sound
#define MAX_BUF_LEN 512
#define LINE_LIMIT 40
#define ERASE_CHAR write(1, "\b \b", 3)

int main(int argc, char *argv[]) {
    char buffer[MAX_BUF_LEN], input_char;
    int cursor_pos, new_pos, saved_pos, idx;
    struct termios original_term, modified_term;

    if (!isatty(0)) {
        perror(argv[0]);
        exit(1);
    }

    if (tcgetattr(0, &original_term) == -1) {
        perror(argv[0]);
        exit(2);
    }

    // Disable canonical input and echoing
    modified_term = original_term;
    modified_term.c_cc[VMIN] = 1;
    modified_term.c_cc[VTIME] = 1;
    modified_term.c_lflag &= ~(ISIG | ICANON | ECHO);

    if (tcsetattr(0, TCSANOW, &modified_term) == -1) {
        perror(argv[0]);
        exit(3);
    }

    cursor_pos = 0;
    while (read(0, &input_char, 1) > 0) {
        // Handle EOF in first position (Ctrl-D)
        if (input_char == CEOT) {
            if (cursor_pos == 0)
                break;
        } 
        // Backspace handling
        else if (input_char == modified_term.c_cc[VERASE]) {
            if (cursor_pos > 0) {
                ERASE_CHAR;
                --cursor_pos;
            }
        }
        // Kill character handling (erase entire line)
        else if (input_char == modified_term.c_cc[VKILL]) {
            while (cursor_pos > 0) {
                ERASE_CHAR;
                --cursor_pos;
            }
        }
        // Ctrl-W to erase until beginning of the last word
        else if (input_char == CWERASE) {
            while (cursor_pos > 0 && isspace(buffer[cursor_pos - 1])) {
                ERASE_CHAR;
                --cursor_pos;
            }
            while (cursor_pos > 0 && !isspace(buffer[cursor_pos - 1])) {
                ERASE_CHAR;
                --cursor_pos;
            }
        }
        // Newline handling (start a new line)
        else if (input_char == '\n') {
            write(1, &input_char, 1);
            cursor_pos = 0;
        }
        // Non-printable characters result in an alert sound
        else if (!isprint(input_char)) {
            write(1, ALERT_SOUND, 1);
        }
        // Write printable character and store it
        else {
            write(1, &input_char, 1);
            buffer[cursor_pos++] = input_char;
        }

        // Optional wrap-around logic
        if (cursor_pos >= LINE_LIMIT && !isspace(input_char)) {
            saved_pos = cursor_pos;

            // Find the beginning of the last word
            while (cursor_pos > 0 && !isspace(buffer[cursor_pos - 1])) {
                --cursor_pos;
            }

            if (cursor_pos > 0) {
                new_pos = 0;

                // Copy last word to the beginning of the line
                for (idx = cursor_pos; idx < saved_pos; idx++) {
                    ERASE_CHAR;
                    buffer[new_pos++] = buffer[idx];
                }
                cursor_pos = new_pos;

                // Display word at the beginning of the line
                write(1, "\n", 1);
                for (idx = 0; idx < cursor_pos; idx++) {
                    write(1, &buffer[idx], 1);
                }
            } else {
                write(1, "\n", 1);
            }
        }
    }

    // Restore terminal settings
    tcsetattr(0, TCSANOW, &original_term);
}
