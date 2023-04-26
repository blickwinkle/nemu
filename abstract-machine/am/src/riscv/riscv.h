#ifndef RISCV_H__
#define RISCV_H__

#include <stdint.h>

static inline uint8_t  inb(uintptr_t addr) { return *(volatile uint8_t  *)addr; }
static inline uint16_t inw(uintptr_t addr) { return *(volatile uint16_t *)addr; }
static inline uint32_t inl(uintptr_t addr) { return *(volatile uint32_t *)addr; }

static inline void outb(uintptr_t addr, uint8_t  data) { *(volatile uint8_t  *)addr = data; }
static inline void outw(uintptr_t addr, uint16_t data) { *(volatile uint16_t *)addr = data; }
static inline void outl(uintptr_t addr, uint32_t data) { *(volatile uint32_t *)addr = data; }

#define PTE_V 0x01
#define PTE_R 0x02
#define PTE_W 0x04
#define PTE_X 0x08
#define PTE_U 0x10
#define PTE_A 0x40
#define PTE_D 0x80

#define CAUSE_MISALIGNED_FETCH 0x0
#define CAUSE_FETCH_ACCESS 0x1
#define CAUSE_ILLEGAL_INSTRUCTION 0x2
#define CAUSE_BREAKPOINT 0x3
#define CAUSE_MISALIGNED_LOAD 0x4
#define CAUSE_LOAD_ACCESS 0x5
#define CAUSE_MISALIGNED_STORE 0x6
#define CAUSE_STORE_ACCESS 0x7
#define CAUSE_USER_ECALL 0x8
#define CAUSE_SUPERVISOR_ECALL 0x9
#define CAUSE_HYPERVISOR_ECALL 0xa
#define CAUSE_MACHINE_ECALL 0xb
#define CAUSE_FETCH_PAGE_FAULT 0xc
#define CAUSE_LOAD_PAGE_FAULT 0xd
#define CAUSE_STORE_PAGE_FAULT 0xf

enum { MODE_U, MODE_S, MODE_M = 3 };
#define MSTATUS_MXR  (1 << 19)
#define MSTATUS_SUM  (1 << 18)

#if __riscv_xlen == 64
#define MSTATUS_SXL  (2ull << 34)
#define MSTATUS_UXL  (2ull << 32)
#else
#define MSTATUS_SXL  0
#define MSTATUS_UXL  0
#endif

#endif
