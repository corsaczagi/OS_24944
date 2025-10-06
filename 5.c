#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

main(int argc, char *argv[])
{
    long displ[500];
    int fd, i = 1, j = 0, line_no, line_ln[500];
    char c, buf[257];
    static char err_msg[32] = "Input file - ";

    if(( fd =  open(argv[1], O_RDONLY)) == -1) {
        perror(strcat(err_msg, argv[1]));
        exit(1);
        }

    displ[1] = 0L;
    while(read(fd, &c, 1))
        if( c == '\n' ) {
            j++;
            line_ln[i++] = j;
            displ[i] = lseek(fd, 0L, 1);
            j = 0;
            }
        else
            j++;

    while( printf("Line number : ") && scanf("%d", &line_no)) {
        if(line_no <= 0)
            exit(0);
        lseek(fd, displ[line_no], 0);
        if(read(fd, buf, line_ln[line_no]))
            write(1, buf, line_ln[line_no]);
        else
            fprintf(stderr, "Bad Line Number\n");
        }
}