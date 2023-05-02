
#include "am.h"
#include "amdev.h"
#include "klib-macros.h"
#include <common.h>
#include <stdint.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for (int i = 0; i < len; i++) {
    putch(((char *)buf)[i]);
  }
  return len;
}

// size_t events_read(void *buf, size_t offset, size_t len) {
//   AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
//   if (ev.keycode == AM_KEY_NONE) {
//     MULTIPROGRAM_YIELD();
//     return 0;
//   }

//   return snprintf(buf, len, "%d %d %s %s\n", ev.keydown, ev.keycode, ev.keydown ? "kdwn" : "kup ", keyname[ev.keycode]);
// }

//offset被忽视
size_t events_read(void *buf, size_t offset, size_t len) {
  //yield();
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) return 0;
  
  // switch (ev.keycode){
  // case AM_KEY_F1:
  //   switch_program_index(1);
  //   return 0;

  // case AM_KEY_F2:
  //   switch_program_index(2);
  //   return 0;

  // case AM_KEY_F3:
  //   switch_program_index(3);
  //   return 0;
  
  // default:
  //   break;
  // }

  //int real_length = 4;
  char *tag = ev.keydown ? "kd " : "ku ";
  //if (real_length <= len){
  strcpy(buf, tag);
  // }else {
  //   assert(0);
  //   return 0;
  // }
  
  //real_length += strlen(keyname[ev.keycode]);
  
  //if (real_length<= len){
  strcat(buf, keyname[ev.keycode]);
  // }else {
  //   Log("Need %d for %s%s but got %d", strlen(keyname[ev.keycode]), (char *)buf, keyname[ev.keycode], len);
  //   assert(0);
  //   return 0;
  // }
  Log("Got  (kbd): %s (%d) %s\n", keyname[ev.keycode], ev.keycode, ev.keydown ? "DOWN" : "UP");
  
  return 1;
}

// 屏幕信息, 包含的keys: `WIDTH`表示宽度, `HEIGHT`表示高度.
size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  //code:
  // 1. 读取屏幕信息
  AM_GPU_CONFIG_T info;
  ioe_read(AM_GPU_CONFIG, &info);
  // printf("dispinfo_read : width: %d, height: %d\n", info.width, info.height);
  // 2. 将信息写入buf
  return snprintf(buf, len, "WIDTH : %d\nHEIGHT : %d\n", info.width, info.height);
}

// 用于把buf中的len字节写到屏幕上offset处. 你需要先从offset计算出屏幕上的坐标, 然后调用IOE来进行绘图. 另外我们约定每次绘图后总是马上将frame buffer中的内容同步到屏幕上.
size_t fb_write(const void *buf, size_t offset, size_t len) {
  //code:
  // AM_GPU_MEMCPY_T info = {
  //   .dest =  offset / sizeof(uint32_t),
  //   .src = buf,
  //   .size = len / sizeof(uint32_t)
  // };
  // ioe_write(AM_GPU_MEMCPY, &info);
  // return len;


  uintptr_t *ptr;
  ptr = (uintptr_t *)(&buf);

  io_write(AM_GPU_MEMCPY, offset, (void *)*ptr, len);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
