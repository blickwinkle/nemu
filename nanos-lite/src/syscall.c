#include <common.h>
#include <stdint.h>
#include <sys/time.h>
#include "am.h"
#include "debug.h"
#include "syscall.h"
void sys_yield(Context *c);
void sys_exit(Context *c);
void sys_write(Context *c);
void sys_brk(Context *c);
void sys_open(Context *c);
void sys_read(Context *c);
void sys_close(Context *c);
void sys_lseek(Context *c);
void sys_gettimeofday(Context *c);
static void ((*syscalls[])(Context *c)) = {
  [SYS_yield] = sys_yield,
  [SYS_exit] = sys_exit,
  [SYS_write] = sys_write,
  [SYS_brk] = sys_brk,
  [SYS_open] = sys_open,
  [SYS_read] = sys_read,
  [SYS_close] = sys_close,
  [SYS_lseek] = sys_lseek,
  [SYS_gettimeofday] = sys_gettimeofday,
};

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  // Log("syscall ID = %d arguments: %x %x %x ret: %x", a[0], c->GPR2, c->GPR3, c->GPR4, c->GPRx);
  switch (a[0]) {
    case SYS_yield: 
    case SYS_exit:
    case SYS_write:
    case SYS_brk:
    case SYS_open:
    case SYS_read:
    case SYS_close:
    case SYS_lseek:
    case SYS_gettimeofday:
      syscalls[a[0]](c); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  
#ifdef CONFIG_STRACE
  Log("syscall ID = %d arguments: %x %x %x ret: %x", a[0], c->GPR2, c->GPR3, c->GPR4, c->GPRx);
#endif
}

void sys_yield(Context *c) {
  c->GPRx = 0;
}

void sys_exit(Context *c) {
  halt(c->GPR2);
}

int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_close(int fd);

void sys_write(Context *c) {
  int fd = c->GPR2;
  char *buf = (char *)c->GPR3;
  int len = c->GPR4;
  // Log("write: %d %p %d", fd, buf, len);
  c->GPRx = fs_write(fd, buf, len);
}

void sys_read(Context *c) {
  int fd = c->GPR2;
  char *buf = (char *)c->GPR3;
  int len = c->GPR4;
  // Log("read: %d %p %d", fd, buf, len);
  c->GPRx = fs_read(fd, buf, len);
}

void sys_lseek(Context *c) {
  int fd = c->GPR2;
  int offset = c->GPR3;
  int whence = c->GPR4;
  c->GPRx = fs_lseek(fd, offset, whence);
}

void sys_open(Context *c) {
  char *pathname = (char *)c->GPR2;
  int flags = c->GPR3;
  int mode = c->GPR4;
  c->GPRx = fs_open(pathname, flags, mode);
}

void sys_close(Context *c) {
  int fd = c->GPR2;
  c->GPRx = fs_close(fd);
}

void sys_brk(Context *c) {
  c->GPRx = 0;
}

void sys_gettimeofday(Context *c) {
  struct timeval *tv = (struct timeval *)c->GPR2;
  __uint64_t time = io_read(AM_TIMER_UPTIME).us;
  tv->tv_usec = (time % 1000000);
  tv->tv_sec = (time / 1000000);
  c->GPRx = 0;
}