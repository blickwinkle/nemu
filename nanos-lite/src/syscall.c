#include <common.h>
#include "syscall.h"
void sys_yield(Context *c);
void sys_exit(Context *c);

static void ((*syscalls[])(Context *c)) = {
  [SYS_yield] = sys_yield,
  [SYS_exit] = sys_exit,
};

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case SYS_yield: 
    case SYS_exit: 
      syscalls[a[0]](c); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

void sys_yield(Context *c) {
  c->GPRx = 0;
}

void sys_exit(Context *c) {
  halt(c->GPR2);
}