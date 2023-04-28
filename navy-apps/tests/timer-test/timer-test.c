#include <stdio.h>
#include <sys/time.h>

int main() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    printf("tv_sec: %ld\n", tv.tv_sec);
    printf("tv_usec: %ld\n", tv.tv_usec);
    //通过gettimeofday()获取当前时间, 并每过0.5秒输出一句话.
    while(1) {
        struct timeval tv1;
        gettimeofday(&tv1, NULL);
        if(tv1.tv_usec - tv.tv_usec >= 500000) {
            printf("tv_sec: %ld\n", tv1.tv_sec);
            printf("tv_usec: %ld\n", tv1.tv_usec);
            tv = tv1;
        }
    }
    return 0;
}