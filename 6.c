#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE 257

main(int argc, char *argv[])
{
    long displ[500];
    int fd1, fd2, i = 1, j = 0, line_no, line_ln[500];
    char c, buf[BUFSIZE];
    static char err_msg[32] = "Input file - ";

    if ((fd1 = open("/dev/tty", O_RDONLY | O_NDELAY)) == -1) {
        perror("/dev/tty");
        exit(1);
    }

    if ((fd2 = open(argv[1], O_RDONLY)) == -1) {
        perror(strcat(err_msg, argv[1]));
        exit(1);
        }

    displ[1] = 0L;
    while(read(fd2, &c, 1))
        if( c == '\n' ) {
            j++;
            line_ln[i++] = j;
            displ[i] = lseek(fd2, 0L, 1);
            j = 0;
            }
        else
            j++;

    for (;;) {
        printf("you have 5 seconds to enter a line number\n");
        sleep(5);
        if ((i = read(fd1, buf, BUFSIZE)) == 0) {
            lseek(fd2, SEEK_SET, 0);
            while((i = read(fd2, buf, BUFSIZE)) > 0)
                write(1, buf, i);
            exit(0);
        }
        else {
            buf[i] = '\0';
            line_no = atoi(buf);
            if(line_no <= 0)
                exit(0);
            lseek(fd2, displ[line_no], 0);
            if(read(fd2, buf, line_ln[line_no]))
                write(1, buf, line_ln[line_no]);
            else
                fprintf(stderr, "Bad Line Number\n");
        }
    }
}