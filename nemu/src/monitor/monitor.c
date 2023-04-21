/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory/paddr.h>
#include <elf.h>
#include <stdio.h>

void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm(const char *triple);

static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
  // Log("Exercise: Please remove me in the source code and compile NEMU again.");
  //assert(0);
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>

void sdb_set_batch_mode();

static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static char *elf_file = NULL;
static int difftest_port = 1234;

#ifdef CONFIG_ITRACE_FUNC
typedef struct ftrace_func {
  uint32_t addr;
  uint32_t size;
  char *name;
  struct ftrace_func *next;
} ftrace_func_t;

static ftrace_func_t *ftrace_func_head = NULL;

static void add_func(const char *name, uint32_t addr, uint32_t size) {
  ftrace_func_t *func = malloc(sizeof(ftrace_func_t));
  func->name = strdup(name);
  func->addr = addr;
  func->size = size;
  func->next = ftrace_func_head;
  ftrace_func_head = func;
}

void free_ftrace_func() {
  ftrace_func_t *func = ftrace_func_head;
  while (func) {
    ftrace_func_t *next = func->next;
    free(func->name);
    free(func);
    func = next;
  }
}


const char *get_func_name(uint32_t addr) {
  ftrace_func_t *func = ftrace_func_head;
  while (func) {
    if (func->addr <= addr && addr < func->addr + func->size) {
      return func->name;
    }
    func = func->next;
  }
  return NULL;
}

static uint32_t call_layer = 0;

void call_ftrace(uint32_t inst_addr, uint32_t next_addr) {
  call_layer++;
  const char *func_name = get_func_name(next_addr);
  if (func_name) {
    char buf[256] = {0};
    sprintf(buf, "%x:", inst_addr);
    for (int i = 0; i < call_layer; i++) {
      strcat(buf, " ");
    }
    char buf2[256] = {0};
    sprintf(buf2, "%s call [%s@%x]", buf, func_name, next_addr);
    Log("%s", buf2);
  }
}

void ret_ftrace(uint32_t inst_addr, uint32_t next_addr) {
  const char *func_name = get_func_name(next_addr);
  if (func_name) {
    char buf[256] = {0};
    sprintf(buf, "%x:", inst_addr);
    for (int i = 0; i < call_layer; i++) {
      strcat(buf, " ");
    }
    char buf2[256] = {0};
    sprintf(buf2, "%s ret [%s@%x]", buf, func_name, next_addr);
    Log("%s", buf2);
  }
  call_layer--;
}

static void init_ftrace(char *elf_file) {

  // 读取elf文件
  // read elf file
  if (elf_file == NULL) {
    Log("No elf file specified, ftrace disabled");
    return;
  }
  FILE *fp = fopen(elf_file, "rb");
  Assert(fp, "Can not open '%s'", elf_file);
  // 解析该elf文件，获取所有函数的开始地址，大小，以及函数名
  // and parse the elf file to get the start address, size and function name of all functions
  Elf32_Ehdr ehdr;
  fread(&ehdr, sizeof(ehdr), 1, fp);
  // 读取symtab
  // read symtab
  fseek(fp, ehdr.e_shoff + ehdr.e_shstrndx * sizeof(Elf32_Shdr), SEEK_SET);
  Elf32_Shdr shstr;
  fread(&shstr, sizeof(shstr), 1, fp);
  fseek(fp, shstr.sh_offset, SEEK_SET);

  char *shstrtab = malloc(shstr.sh_size);

  fread(shstrtab, shstr.sh_size, 1, fp);
  fseek(fp, ehdr.e_shoff, SEEK_SET);
  for (int i = 0; i < ehdr.e_shnum; i++) {
    Elf32_Shdr shdr;
    fread(&shdr, sizeof(shdr), 1, fp);
    if (strcmp(shstrtab + shdr.sh_name, ".symtab") == 0) {
      // 读取symtab
      // read symtab
      fseek(fp, shdr.sh_offset, SEEK_SET);
      int nr_symtab = shdr.sh_size / sizeof(Elf32_Sym);
      Elf32_Sym *symtab = malloc(shdr.sh_size);
      fread(symtab, shdr.sh_size, 1, fp);
      // 读取strtab
      // read strtab
      fseek(fp, ehdr.e_shoff + shdr.sh_link * sizeof(Elf32_Shdr), SEEK_SET);
      Elf32_Shdr str_shdr;
      fread(&str_shdr, sizeof(str_shdr), 1, fp);
      fseek(fp, str_shdr.sh_offset, SEEK_SET);
      // int nr_strtab = str_shdr.sh_size;
      char *strtab = malloc(str_shdr.sh_size);
      fread(strtab, str_shdr.sh_size, 1, fp);
      // 读取所有函数的信息
      // read the information of all functions
      for (int i = 0; i < nr_symtab; i++) {
        if (ELF32_ST_TYPE(symtab[i].st_info) == STT_FUNC) {
          // 读取函数名
          // read function name
          char *name = strtab + symtab[i].st_name;
          // 读取函数的开始地址
          // read the start address of the function
          paddr_t addr = symtab[i].st_value;
          // 读取函数的大小
          // read the size of the function
          size_t size = symtab[i].st_size;
          // 将函数的信息添加到函数列表中
          // add the information of the function to the function list
          add_func(name, addr, size);
          // 打印一下
          // print it
          Log("Function '%s' is loaded at 0x%08x, size = %zu", name, addr, size);
        }
      }
      free(symtab);
      free(strtab);
      break ;
    }
  }
  free(shstrtab);
  fclose(fp);
  
}
#endif // CONFIG_ITRACE_FUNC

static long load_img() {
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
  }

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  Log("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);

  fclose(fp);

  return size;
}

static int parse_args(int argc, char *argv[]) {
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"help"     , no_argument      , NULL, 'h'},
    {"elf_file" , required_argument, NULL, 'e'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:d:p:e:", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 'e': elf_file = optarg; break;
      case 1: img_file = optarg; return 0;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\t-e,--elf_file=FILE      used by ftrace\n");
        printf("\n");
        exit(0);
    }
  }
  return 0;
}

void init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. */

  /* Parse arguments. */
  parse_args(argc, argv);

  /* Set random seed. */
  init_rand();

  /* Open the log file. */
  init_log(log_file);

  /* Initialize memory. */
  init_mem();

  /* Initialize devices. */
  IFDEF(CONFIG_DEVICE, init_device());

  /* Perform ISA dependent initialization. */
  init_isa();

  /* Load the image to memory. This will overwrite the built-in image. */
  long img_size = load_img();

  /* Initialize differential testing. */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* Initialize the simple debugger. */
  init_sdb();

  IFDEF(CONFIG_ITRACE, init_disasm(
    MUXDEF(CONFIG_ISA_x86,     "i686",
    MUXDEF(CONFIG_ISA_mips32,  "mipsel",
    MUXDEF(CONFIG_ISA_riscv32, "riscv32",
    MUXDEF(CONFIG_ISA_riscv64, "riscv64", "bad")))) "-pc-linux-gnu"
  ));

  IFDEF(CONFIG_ITRACE_FUNC, init_ftrace(elf_file));

  /* Display welcome message. */
  welcome();
}
#else // CONFIG_TARGET_AM
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif
