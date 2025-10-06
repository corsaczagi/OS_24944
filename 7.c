#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

main(int argc, char *argv[])
{
    static char *displ[501];
    char *p, buf[10];
    int fd1, fd2, count, i = 1, j = 1, line_no, line_ln[500];
    static char err_msg[32] = "Input file - ";
    off_t size;

    if ((fd1 = open("/dev/tty", O_RDONLY | O_NDELAY)) == -1) {
        perror("/dev/tty");
        exit(1);
    }

    if ((fd2 = open(argv[1], O_RDONLY)) == -1) {
        perror(strcat(err_msg, argv[1]));
        exit(1);
        }

    size = lseek(fd2, 0, SEEK_END);
    p = mmap(0, size, PROT_READ, MAP_SHARED, fd2, 0);

    displ[1] = p;
    for(count = 0; count < size; count++)
        if( *(p+count) == '\n' ) {
            line_ln[i++] = j;
            displ[i] = count+p+1;
            j = 1;
            }
        else
            j++;

    displ[i] = 0;
    for (;;) {
        printf("you have 5 seconds to enter a line number\n");
        sleep(5);
        if ((i = read(fd1, buf, 10)) == 0) {
            write(1, p, size);
            exit(0);
        }
        else {
            buf[i] = '\0';
            line_no = atoi(buf);
            if(line_no <= 0)
                exit(0);
            if(displ[line_no] != 0)
                write(1, displ[line_no], line_ln[line_no]);
            else
                fprintf(stderr, "Bad Line Number\n");
        }
    }
}