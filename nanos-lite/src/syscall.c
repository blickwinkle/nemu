#include <common.h>
#include "am.h"
#include "syscall.h"
void sys_yield(Context *c);
void sys_exit(Context *c);
void sys_write(Context *c);
void sys_brk(Context *c);
static void ((*syscalls[])(Context *c)) = {
  [SYS_yield] = sys_yield,
  [SYS_exit] = sys_exit,
  [SYS_write] = sys_write,
  [SYS_brk] = sys_brk,
};

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case SYS_yield: 
    case SYS_exit:
    case SYS_write:
    case SYS_brk:
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

// 检查fd的值, 如果fd是1或2(分别代表stdout和stderr), 则将buf为首地址的len字节输出到串口(使用putch()即可). 最后还要设置正确的返回值

void sys_write(Context *c) {
  int fd = c->GPR2;
  char *buf = (char *)c->GPR3;
  int len = c->GPR4;
  if (fd == 1 || fd == 2) {
    for (int i = 0; i < len; i++) {
      putch(buf[i]);
    }
    c->GPRx = len;
  } else {
    panic("sys_write: fd = %d", fd);
  }
}

void sys_brk(Context *c) {
  c->GPRx = 0;
}