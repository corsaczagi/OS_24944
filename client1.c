#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK1 "/tmp/sock30.sock"
#define SOCK2 "/tmp/sock31.sock"
#define SOCK3 "/tmp/sock32.sock"

int main() {
    int fd;
    struct sockaddr_un addr;
    const char *path = NULL;

    printf("Выберите сервер:\n");
    printf("1 — сервер A (%s)\n", SOCK1);
    printf("2 — сервер B (%s)\n", SOCK2);
    printf("3 — сервер C (%s)\n", SOCK3);
    printf("> ");

    int choice = getchar();
    getchar(); // удалить \n из буфера

    if (choice == '1') path = SOCK1;
    else if (choice == '2') path = SOCK2;
    else if (choice == '3') path = SOCK3;
    else {
        printf("Неизвестный выбор. Завершение.\n");
        return 1;
    }

    // создаём сокет
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        exit(1);
    }

    // структура адреса
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    printf("Подключение к серверу: %s\n", path);

    // подключаемся
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        exit(1);
    }

    printf("Введите текст (Ctrl+D для завершения):\n");

    char buf[1024];
    while (fgets(buf, sizeof(buf), stdin)) {
        write(fd, buf, strlen(buf));
    }

    close(fd);
    return 0;
}
