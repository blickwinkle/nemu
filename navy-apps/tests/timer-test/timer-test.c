#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <NDL.h>

int main() {
    uint32_t start = NDL_GetTicks();
    //NDL_GetTicks()获取当前时间, 并每过0.5秒输出一句话.
    while(1) {
        uint32_t now = NDL_GetTicks();
        if (now - start >= 500) {
            printf("Hello, world!\n");
            start = now;
        }
    }
    return 0;
}