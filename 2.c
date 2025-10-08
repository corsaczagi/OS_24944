#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
    time_t t;
    struct tm *tm;
    
    setenv("TZ", "PST8PDT", 1);
    tzset();
    
    time(&t);
    printf("%s", ctime(&t));
    
    tm = localtime(&t);
    printf("%d/%d/%02d %d:%02d %s\n",
           tm->tm_mon + 1, tm->tm_mday,
           tm->tm_year + 1900, tm->tm_hour,
           tm->tm_min, tzname[tm->tm_isdst]);
    
    return 0;
}