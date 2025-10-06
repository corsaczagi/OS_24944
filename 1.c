#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int c;
    struct rlimit rlp;
    extern char **environ;

    if(argc < 2) {
        fprintf(stderr, "Usage: %s options\n", argv[0]);
        fprintf(stderr, "Options: -i (user info), -s (setpgid), -p (process info)\n");
        fprintf(stderr, "         -u (show ulimit), -U val (set ulimit)\n");
        fprintf(stderr, "         -c (show core limit), -C val (set core limit)\n");
        fprintf(stderr, "         -d (current directory), -v (environment)\n");
        fprintf(stderr, "         -V var=val (set environment variable)\n");
        return 1;
    }

    while ((c = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
        switch (c) {
            case 'i': 
                printf("UID:%d/%d GID:%d/%d\n", 
                       getuid(), geteuid(), 
                       getgid(), getegid());
                break;
                
            case 's': 
                if (setpgid(0, 0) == -1) {
                    fprintf(stderr, "setpgid error: %s\n", strerror(errno));
                }
                break;
                
            case 'p': 
                printf("PID:%d PPID:%d PGID:%d\n", 
                       getpid(), getppid(), getpgid(0));
                break;
                
            case 'U': 
                if (ulimit(2, atol(optarg)) == -1) {
                    fprintf(stderr, "ulimit error: %s\n", strerror(errno));
                }
                break;
                
            case 'u': 
                printf("ulimit=%ld\n", ulimit(1, 0));
                break;
                
            case 'c': 
                if (getrlimit(RLIMIT_CORE, &rlp) == -1) {
                    fprintf(stderr, "getrlimit error: %s\n", strerror(errno));
                } else {
                    printf("core=%ld\n", (long)rlp.rlim_cur);
                }
                break;
                
            case 'C': 
                if (getrlimit(RLIMIT_CORE, &rlp) == -1) {
                    fprintf(stderr, "getrlimit error: %s\n", strerror(errno));
                    break;
                }
                rlp.rlim_cur = atol(optarg);
                if (setrlimit(RLIMIT_CORE, &rlp) == -1) {
                    fprintf(stderr, "setrlimit error: %s\n", strerror(errno));
                }
                break;
                
            case 'd': {
                char *cwd = getcwd(NULL, 0);
                if (cwd == NULL) {
                    fprintf(stderr, "getcwd error: %s\n", strerror(errno));
                } else {
                    printf("CWD:%s\n", cwd);
                    free(cwd);
                }
                break;
            }
                
            case 'v': 
                for(char **p = environ; *p; p++) {
                    printf("%s\n", *p);
                }
                break;
                
            case 'V': 
                if (putenv(optarg) != 0) {
                    fprintf(stderr, "putenv error: %s\n", strerror(errno));
                }
                break;
                
            case '?':
                fprintf(stderr, "Unknown option: -%c\n", optopt);
                fprintf(stderr, "Use valid options: ispuU:cC:dvV:\n");
                return 1;
                
            default:
                fprintf(stderr, "Unexpected error processing option: -%c\n", c);
                return 1;
        }
    }

    // Проверка на лишние аргументы
    if (optind < argc) {
        fprintf(stderr, "Unexpected arguments: ");
        while (optind < argc) {
            fprintf(stderr, "%s ", argv[optind++]);
        }
        fprintf(stderr, "\n");
        return 1;
    }

    return 0;
}