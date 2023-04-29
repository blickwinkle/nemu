#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
  // 1. 用libc中的文件操作打开文件, 并获取文件大小size
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    printf("IMG_Load: failed to open file %s\n", filename);
    return NULL;
  }
  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  // 2. 申请一段大小为size的内存区间buf
  char *buf = malloc(size);
  // 3. 将文件中的内容读入到buf中
  fread(buf, size, 1, fp);
  // 4.将buf和size作为参数, 调用STBIMG_LoadFromMemory(), 它会返回一个SDL_Surface结构的指针
  SDL_Surface *ret = STBIMG_LoadFromMemory(buf, size);
  // 5. 关闭文件, 释放申请的内存
  fclose(fp);
  free(buf);
  // 6. 返回SDL_Surface结构的指针
  return ret;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
