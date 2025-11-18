#include <stdio.h>
#include <signal.h>
#define   BEL   '\07'

int count;

main()
{
    void sigcatch(int);

    signal(SIGINT, sigcatch);
    signal(SIGQUIT, sigcatch);

    for (;;)
        pause();
}

void sigcatch(int sig)
{
    if (sig == SIGQUIT) {
        printf("bell was rung %d times\n", count);
        exit(1);
    }
    printf("%c\n", BEL);
    count++;
    signal(sig, sigcatch);
}
