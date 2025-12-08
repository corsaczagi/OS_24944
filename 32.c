#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "/tmp/sock32.sock"
#define MAX_CLIENTS 64
#define BUF_SIZE 1024

int server_fd;
int clients[MAX_CLIENTS];

void set_async(int fd) {
    int flags;

    flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK | O_ASYNC);

    fcntl(fd, F_SETOWN, getpid());
}

void sigio_handler(int signo) {
    char buf[BUF_SIZE];

    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd >= 0) {
        printf("Новый клиент: %d\n", client_fd);

        set_async(client_fd);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] == -1) {
                clients[i] = client_fd;
                break;
            }
        }
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        int fd = clients[i];
        if (fd == -1) continue;

        int n = read(fd, buf, BUF_SIZE);
        if (n > 0) {
            for (int j = 0; j < n; j++)
                buf[j] = toupper((unsigned char)buf[j]);

            write(STDOUT_FILENO, buf, n);
        }
        else if (n == 0) {
            printf("Клиент %d отключился\n", fd);
            close(fd);
            clients[i] = -1;
        }
    }
}

int main() {
    struct sockaddr_un addr;

    for (int i = 0; i < MAX_CLIENTS; i++)
        clients[i] = -1;

    unlink(SOCK_PATH);

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);

    printf("Асинхронный сервер запущен...\n");

    set_async(server_fd);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigio_handler;
    sigaction(SIGIO, &sa, NULL);

    while (1) {
        pause();
    }

    close(server_fd);
    unlink(SOCK_PATH);
    return 0;
}