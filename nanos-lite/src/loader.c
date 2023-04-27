#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

// 从ramdisk中`offset`偏移处的`len`字节读入到`buf`中
size_t ramdisk_read(void *buf, size_t offset, size_t len);

// 把`buf`中的`len`字节写入到ramdisk中`offset`偏移处
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

// 返回ramdisk的大小, 单位为字节
size_t get_ramdisk_size();

static uintptr_t loader(PCB *pcb, const char *filename) {

  //你需要找出每一个需要加载的segment的Offset, VirtAddr, FileSiz和MemSiz这些参数. 其中相对文件偏移Offset指出相应segment的内容从ELF文件的第Offset字节开始, 在文件中的大小为FileSiz, 它需要被分配到以VirtAddr为首地址的虚拟内存位置, 在内存中它占用大小为MemSiz. 也就是说, 这个segment使用的内存就是[VirtAddr, VirtAddr + MemSiz)这一连续区间, 然后将segment的内容从ELF文件中读入到这一内存区间, 并将[VirtAddr + FileSiz, VirtAddr + MemSiz)对应的物理区间清零.
  // code:
  Elf_Ehdr elf;
  ramdisk_read(&elf, 0, sizeof(Elf_Ehdr));
  // 检查ELF文件的魔数
  assert(*(uint32_t *)elf.e_ident == 0x464c457f);
  // // 检查ISA
  // assert(elf.e_machine == EXPECT_TYPE);
  Elf_Phdr ph;
  ramdisk_read(&ph, elf.e_phoff, sizeof(Elf_Phdr));
  for (int i = 0; i < elf.e_phnum; i++) {
    if (ph.p_type == PT_LOAD) {
      // 从ELF文件中读取seg
      //code:
      ramdisk_read((void *)ph.p_vaddr, ph.p_offset, ph.p_filesz);
      memset((void *)(ph.p_vaddr + ph.p_filesz), 0, ph.p_memsz - ph.p_filesz);
    }
    ramdisk_read(&ph, elf.e_phoff + (i + 1) * sizeof(Elf_Phdr), sizeof(Elf_Phdr));
  }
  return elf.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

