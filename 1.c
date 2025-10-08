#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <ulimit.h>
#include <stdlib.h>
#include <stdio.h>

#define GET_LIM 1
#define SET_LIM 2

extern char **environ;

int main(int argc, char *argv[]) {
    int c;
    struct rlimit r;
    char **p;
    
    if(argc < 2) {
        fprintf(stderr,"Use: %s options\n", argv[0]);
        return 1;
    }
    
    while((c = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
        switch(c) {
        case 'i':
            printf("uid=%ld euid=%ld gid=%ld egid=%ld\n", 
                   getuid(), geteuid(), getgid(), getegid());
            break;
        case 's':
            setpgid(0,0);
            break;
        case 'p':
            printf("pid=%ld ppid=%ld pgid=%ld\n", 
                   getpid(), getppid(), getpgid(0));
            break;
        case 'U':
            if(optarg == NULL) {
                fprintf(stderr, "Option -U requires argument\n");
                break;
            }
            if(ulimit(SET_LIM, atol(optarg)) == -1)
                fprintf(stderr,"Need root for ulimit\n");
            break;
        case 'u':
            printf("ulimit=%ld\n", ulimit(GET_LIM,0));
            break;
        case 'c':
            getrlimit(RLIMIT_CORE, &r);
            printf("core=%ld\n", r.rlim_cur);
            break;
        case 'C':
            if(optarg == NULL) {
                fprintf(stderr, "Option -C requires argument\n");
                break;
            }
            getrlimit(RLIMIT_CORE, &r);
            r.rlim_cur = atol(optarg);
            if(setrlimit(RLIMIT_CORE, &r) == -1)
                fprintf(stderr,"Need root for core\n");
            break;
        case 'd':
            printf("cwd=%s\n", getcwd(NULL,0));
            break;
        case 'v':
            for(p=environ; *p; p++) printf("%s\n", *p);
            break;
        case 'V':
            if(optarg == NULL) {
                fprintf(stderr, "Option -V requires argument\n");
                break;
            }
            putenv(optarg);
            break;
        case '?':
            fprintf(stderr, "Unknown option: -%c\n", optopt);
            break;
        }
    }
    return 0;
}