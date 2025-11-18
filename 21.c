#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int sigint_count = 0;

void sigint_handler(int sig) {
    sigint_count++;
    putchar(7);
    fflush(stdout);
}

void sigquit_handler(int sig) {
    printf("\nReceived SIGQUIT. SIGINT signal was received %d times.\n", sigint_count);
    exit(0);
}

int main() {
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigquit_handler);

    while (1) {
        pause();
    }

    return 0;
}