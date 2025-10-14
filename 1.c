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
#include <string.h>

extern char **environ;

static void print_usage(const char *prog) {
    fprintf(stderr,
        "Use: %s options\n"
        "  -i          Print uid/euid gid/egid\n"
        "  -s          setpgid(0,0)\n"
        "  -p          Print pid/ppid/pgid\n"
        "  -u          Print RLIMIT_NPROC (soft) if supported\n"
        "  -U N        Set RLIMIT_NPROC (soft) to N (<= hard) if supported\n"
        "  -c          Print RLIMIT_CORE (soft)\n"
        "  -C N        Set RLIMIT_CORE (soft) to N bytes\n"
        "  -n          Print RLIMIT_NOFILE (soft)\n"
        "  -N N        Set RLIMIT_NOFILE (soft) to N (<= hard)\n"
        "  -d          Print current working directory\n"
        "  -v          Print environment\n"
        "  -V NAME=VAL putenv(NAME=VAL)\n",
        prog);
}

static int parse_ull(const char *s, unsigned long long *out) {
    if (!s || !*s) return -1;
    errno = 0;
    char *end = NULL;
    unsigned long long v = strtoull(s, &end, 10);
    if (errno || end == s || *end != '\0') return -1;
    *out = v;
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    int c;
    while ((c = getopt(argc, argv, "ispuU:cC:dvV:nN:")) != -1) {
        switch (c) {
        case 'i':
            printf("uid=%ld euid=%ld gid=%ld egid=%ld\n",
                   (long)getuid(), (long)geteuid(),
                   (long)getgid(), (long)getegid());
            break;

        case 's':
            if (setpgid(0, 0) == -1) perror("setpgid");
            break;

        case 'p': {
            pid_t pid = getpid();
            pid_t ppid = getppid();
            pid_t pgid = getpgid(0);
            if (pgid == -1) perror("getpgid");
            printf("pid=%ld ppid=%ld pgid=%ld\n",
                   (long)pid, (long)ppid, (long)pgid);
            break;
        }

        case 'U': {
#ifndef RLIMIT_NPROC
            (void)optarg;
            fprintf(stderr, "RLIMIT_NPROC is not supported on this system.\n");
#else
            if (!optarg) { fprintf(stderr, "-U needs a number\n"); break; }
            struct rlimit r;
            if (getrlimit(RLIMIT_NPROC, &r) == -1) { perror("getrlimit(RLIMIT_NPROC)"); break; }
            unsigned long long want;
            if (parse_ull(optarg, &want) != 0) { fprintf(stderr, "Invalid number for -U\n"); break; }
            uintmax_t hard = (r.rlim_max == RLIM_INFINITY) ? UINTMAX_MAX : (uintmax_t)r.rlim_max;
            uintmax_t newsoft = (want > hard) ? hard : (uintmax_t)want;
            r.rlim_cur = (rlim_t)newsoft;
            if (setrlimit(RLIMIT_NPROC, &r) == -1) perror("setrlimit(RLIMIT_NPROC)");
            else printf("RLIMIT_NPROC soft set to %ju (hard=%ju)\n",
                        (uintmax_t)r.rlim_cur,
                        (r.rlim_max == RLIM_INFINITY) ? (uintmax_t)UINTMAX_MAX : (uintmax_t)r.rlim_max);
#endif
            break;
        }

        case 'u': {
#ifndef RLIMIT_NPROC
            fprintf(stderr, "RLIMIT_NPROC is not supported on this system.\n");
#else
            struct rlimit r;
            if (getrlimit(RLIMIT_NPROC, &r) == -1) perror("getrlimit(RLIMIT_NPROC)");
            else if (r.rlim_cur == RLIM_INFINITY) puts("nproc=unlimited");
            else printf("%ju\n", (uintmax_t)r.rlim_cur);
#endif
            break;
        }

        case 'c': {
            struct rlimit r;
            if (getrlimit(RLIMIT_CORE, &r) == -1) perror("getrlimit(RLIMIT_CORE)");
            else if (r.rlim_cur == RLIM_INFINITY) puts("core=unlimited");
            else printf("core=%ju\n", (uintmax_t)r.rlim_cur);
            break;
        }

        case 'C': {
            if (!optarg) { fprintf(stderr, "-C needs a number\n"); break; }
            struct rlimit r;
            if (getrlimit(RLIMIT_CORE, &r) == -1) { perror("getrlimit(RLIMIT_CORE)"); break; }
            unsigned long long want;
            if (parse_ull(optarg, &want) != 0) { fprintf(stderr, "Invalid number for -C\n"); break; }
            uintmax_t hard = (r.rlim_max == RLIM_INFINITY) ? UINTMAX_MAX : (uintmax_t)r.rlim_max;
            uintmax_t newsoft = (want > hard) ? hard : (uintmax_t)want;
            r.rlim_cur = (rlim_t)newsoft;
            if (setrlimit(RLIMIT_CORE, &r) == -1) perror("setrlimit(RLIMIT_CORE)");
            else printf("RLIMIT_CORE soft set to %ju (hard=%ju)\n",
                        (uintmax_t)r.rlim_cur,
                        (r.rlim_max == RLIM_INFINITY) ? (uintmax_t)UINTMAX_MAX : (uintmax_t)r.rlim_max);
            break;
        }

        case 'n': { // show RLIMIT_NOFILE
            struct rlimit r;
            if (getrlimit(RLIMIT_NOFILE, &r) == -1) perror("getrlimit(RLIMIT_NOFILE)");
            else if (r.rlim_cur == RLIM_INFINITY) puts("nofile=unlimited");
            else printf("nofile=%ju\n", (uintmax_t)r.rlim_cur);
            break;
        }

        case 'N': { // set RLIMIT_NOFILE soft limit
            if (!optarg) { fprintf(stderr, "-N needs a number\n"); break; }
            struct rlimit r;
            if (getrlimit(RLIMIT_NOFILE, &r) == -1) { perror("getrlimit(RLIMIT_NOFILE)"); break; }
            unsigned long long want;
            if (parse_ull(optarg, &want) != 0) { fprintf(stderr, "Invalid number for -N\n"); break; }
            uintmax_t hard = (r.rlim_max == RLIM_INFINITY) ? UINTMAX_MAX : (uintmax_t)r.rlim_max;
            uintmax_t newsoft = (want > hard) ? hard : (uintmax_t)want;
            r.rlim_cur = (rlim_t)newsoft;
            if (setrlimit(RLIMIT_NOFILE, &r) == -1) perror("setrlimit(RLIMIT_NOFILE)");
            else printf("RLIMIT_NOFILE soft set to %ju (hard=%ju)\n",
                        (uintmax_t)r.rlim_cur,
                        (r.rlim_max == RLIM_INFINITY) ? (uintmax_t)UINTMAX_MAX : (uintmax_t)r.rlim_max);
            break;
        }

        case 'd': {
            char *cwd = getcwd(NULL, 0);
            if (!cwd) perror("getcwd");
            else { printf("cwd=%s\n", cwd); free(cwd); }
            break;
        }

        case 'v': {
            for (char **p = environ; *p; ++p) puts(*p);
            break;
        }

        case 'V': {
            if (!optarg) { fprintf(stderr, "-V needs NAME=VALUE\n"); break; }
            if (!strchr(optarg, '=')) { fprintf(stderr, "-V needs NAME=VALUE\n"); break; }
            if (putenv(optarg) != 0) perror("putenv");
            break;
        }

        default:
            fprintf(stderr, "Unknown option: -%c\n", optopt ? optopt : c);
            print_usage(argv[0]);
            return 1;
        }
    }

    return 0;
}
