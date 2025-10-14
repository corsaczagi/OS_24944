#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <ulimit.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>

extern char **environ;

int main(int argc, char *argv[]) {
    int c;
    struct rlimit r;

    if (argc < 2) {
        fprintf(stderr, "Use: %s options\n", argv[0]);
        return 1;
    }

    while ((c = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
        switch (c) {
        case 'i':
            printf("uid=%ld euid=%ld gid=%ld egid=%ld\n",
                   (long)getuid(), (long)geteuid(),
                   (long)getgid(), (long)getegid());
            break;

        case 's':
            if (setpgid(0, 0) == -1) perror("setpgid");
            break;

        case 'p':
            printf("pid=%ld ppid=%ld pgid=%ld\n",
                   (long)getpid(), (long)getppid(), (long)getpgid(0));
            break;

        case 'U': {
#ifndef RLIMIT_NPROC
            fprintf(stderr, "RLIMIT_NPROC is not supported on this system.\n");
#else
            if (!optarg) { fprintf(stderr, "-U needs an argument\n"); break; }
            if (getrlimit(RLIMIT_NPROC, &r) == -1) { perror("getrlimit"); break; }
            r.rlim_cur = (rlim_t)atoll(optarg);
            if (setrlimit(RLIMIT_NPROC, &r) == -1) perror("setrlimit");
#endif
            break; }

        case 'u': {
#ifndef RLIMIT_NPROC
            fprintf(stderr, "RLIMIT_NPROC is not supported on this system.\n");
#else
            if (getrlimit(RLIMIT_NPROC, &r) == -1) perror("getrlimit");
            else if (r.rlim_cur == RLIM_INFINITY) puts("ulimit=-1 (unlimited)");
            else printf("ulimit=%ju\n", (uintmax_t)r.rlim_cur);
#endif
            break; }

        case 'c':
            if (getrlimit(RLIMIT_CORE, &r) == -1) perror("getrlimit");
            else if (r.rlim_cur == RLIM_INFINITY) puts("core=unlimited");
            else printf("core=%ju\n", (uintmax_t)r.rlim_cur);
            break;

        case 'C':
            if (!optarg) { fprintf(stderr, "-C needs an argument\n"); break; }
            if (getrlimit(RLIMIT_CORE, &r) == -1) { perror("getrlimit"); break; }
            r.rlim_cur = (rlim_t)atoll(optarg);
            if (setrlimit(RLIMIT_CORE, &r) == -1) perror("setrlimit");
            break;

        case 'd': {
            char *cwd = getcwd(NULL, 0);
            if (!cwd) perror("getcwd");
            else { printf("cwd=%s\n", cwd); free(cwd); }
            break; }

        case 'v': {
            for (char **p = environ; *p; ++p) puts(*p);
            break; }

        case 'V':
            if (!optarg) { fprintf(stderr, "-V needs NAME=VALUE\n"); break; }
            if (!strchr(optarg, '=')) { fprintf(stderr, "-V needs NAME=VALUE\n"); break; }
            if (putenv(optarg) != 0) perror("putenv");
            break;

        default:
            fprintf(stderr, "Unknown option: -%c\n", optopt ? optopt : c);
            return 1;
        }
    }
    return 0;
}