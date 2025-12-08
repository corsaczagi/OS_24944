#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/select.h>

#define SOCK_PATH "/tmp/sock31.sock"
#define BUF_SIZE 1024

int main() {
    int server_fd, client_fd, max_fd, n;
    struct sockaddr_un addr;

    fd_set readfds;
    int clients[FD_SETSIZE];

    unlink(SOCK_PATH);

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path)-1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(1);
    }

    printf("Сервер запущен и ждёт клиентов...\n");

    for (int i = 0; i < FD_SETSIZE; i++)
        clients[i] = -1;

    max_fd = server_fd;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        for (int i = 0; i < FD_SETSIZE; i++) {
            if (clients[i] >= 0)
                FD_SET(clients[i], &readfds);
        }

        int ready = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (ready < 0) {
            perror("select");
            continue;
        }

        if (FD_ISSET(server_fd, &readfds)) {
            client_fd = accept(server_fd, NULL, NULL);
            if (client_fd < 0) {
                perror("accept");
                continue;
            }

            int i;
            for (i = 0; i < FD_SETSIZE; i++) {
                if (clients[i] < 0) {
                    clients[i] = client_fd;
                    break;
                }
            }

            if (i == FD_SETSIZE) {
                printf("Слишком много клиентов\n");
                close(client_fd);
            } else {
                if (client_fd > max_fd)
                    max_fd = client_fd;

                printf("Клиент подключился: fd=%d\n", client_fd);
            }
        }

        for (int i = 0; i < FD_SETSIZE; i++) {
            int fd = clients[i];

            if (fd >= 0 && FD_ISSET(fd, &readfds)) {
                char buf[BUF_SIZE];

                n = read(fd, buf, BUF_SIZE);
                if (n <= 0) {
                    printf("Клиент отключился: fd=%d\n", fd);
                    close(fd);
                    clients[i] = -1;
                    continue;
                }

                buf[n] = '\0';

                if (strncmp(buf, "/quit", 5) == 0){
                    printf("Завершение сервера.\n");
                    goto end;
                }

                for (int j = 0; j < n; j++)
                    buf[j] = toupper((unsigned char)buf[j]);

                write(STDOUT_FILENO, buf, n);
            }
        }
    }

    end:
        close(server_fd);
        unlink(SOCK_PATH);
        return 0;

    close(server_fd);
    unlink(SOCK_PATH);
    return 0;
}
