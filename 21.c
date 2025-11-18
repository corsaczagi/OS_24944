#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int signal_count;

void handle_sigint(int sig) {
    signal_count++;
    putchar(7);  // Звуковой сигнал
    fflush(stdout);  // Немедленный вывод
}

void handle_sigquit(int sig) {
    printf("\nSIGQUIT received. SIGINT was triggered %d times.\n", signal_count);
    exit(0);
}

int main() {
    if (signal(SIGINT, handle_sigint) == SIG_ERR) {
        perror("Failed to handle SIGINT");
        exit(1);
    }

    if (signal(SIGQUIT, handle_sigquit) == SIG_ERR) {
        perror("Failed to handle SIGQUIT");
        exit(1);
    }

    printf("Program is running. Press Ctrl+C to trigger SIGINT and Ctrl+\\ to trigger SIGQUIT.\n");

    while (1) {
        pause();  // Ожидаем сигналы
    }

    return 0;
}
