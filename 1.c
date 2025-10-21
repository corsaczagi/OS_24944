// result1-mini.c
#define _GNU_SOURCE
#include <sys/resource.h>
#include <unistd.h>
#include <ulimit.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

extern char **environ;

static long num(const char *s){ char *e; errno=0; long v=strtol(s,&e,10); return (!errno && *e=='\0')?v:-1; }

static void use(const char *p){
    fprintf(stderr,"usage: %s [-i] [-s] [-p] [-u] [-U n] [-c] [-C n] [-d] [-v] [-V name=val]\n",p);
}

int main(int ac, char **av){
    if (ac<2){ use(av[0]); return 2; }

    int opt; struct rlimit r;
    while ((opt=getopt(ac,av,"ispuU:cC:dvV:"))!=-1){
        switch(opt){
        case 'i':
            printf("uid=%ld euid=%ld gid=%ld egid=%ld\n",
                   (long)getuid(),(long)geteuid(),(long)getgid(),(long)getegid());
            break;
        case 's':
            if (setpgid(0,0)==-1) perror("setpgid");
            break;
        case 'p':
            printf("pid=%ld ppid=%ld pgid=%ld\n",
                   (long)getpid(),(long)getppid(),(long)getpgid(0));
            break;
        case 'u':
            if (getrlimit(RLIMIT_NPROC,&r)==0) printf("nproc=%ld\n",(long)r.rlim_cur);
            else perror("getrlimit");
            break;
        case 'U': {
            long v=num(optarg); if (v<0){ fputs("-U bad\n",stderr); return 3; }
            if (getrlimit(RLIMIT_NPROC,&r)==0){ r.rlim_cur=(rlim_t)v; if (setrlimit(RLIMIT_NPROC,&r)==-1) perror("setrlimit"); }
            else perror("getrlimit");
        } break;
        case 'c':
            if (getrlimit(RLIMIT_CORE,&r)==0) printf("core=%ld\n",(long)r.rlim_cur);
            else perror("getrlimit");
            break;
        case 'C': {
            long v=num(optarg); if (v<0){ fputs("-C bad\n",stderr); return 3; }
            if (getrlimit(RLIMIT_CORE,&r)==0){ r.rlim_cur=(rlim_t)v; if (setrlimit(RLIMIT_CORE,&r)==-1) perror("setrlimit"); }
            else perror("getrlimit");
        } break;
        case 'd': {
            char *cwd=getcwd(NULL,0);
            if (!cwd) perror("getcwd"); else { printf("cwd=%s\n",cwd); free(cwd); }
        } break;
        case 'v':
            for (char **p=environ; *p; ++p) puts(*p);
            break;
        case 'V':
            if (!strchr(optarg,'=')) { fputs("-V name=val\n",stderr); return 4; }
            if (putenv(optarg)!=0) perror("putenv");
            break;
        default:
            use(av[0]); return 1;
        }
    }
    return 0;
}
