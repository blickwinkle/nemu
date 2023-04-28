
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

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) {
    MULTIPROGRAM_YIELD();
    return 0;
  }

  return snprintf(buf, len, "%s %s\n", ev.keydown ? "kdwn" : "kup ", keyname[ev.keycode]);
}
// 屏幕信息, 包含的keys: `WIDTH`表示宽度, `HEIGHT`表示高度.
size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  //code:
  // 1. 读取屏幕信息
  AM_GPU_CONFIG_T info;
  ioe_read(AM_GPU_CONFIG, &info);
  printf("dispinfo_read : width: %d, height: %d\n", info.width, info.height);
  // 2. 将信息写入buf
  return snprintf(buf, len, "%d\n%d\n", info.width, info.height);
}

// 用于把buf中的len字节写到屏幕上offset处. 你需要先从offset计算出屏幕上的坐标, 然后调用IOE来进行绘图. 另外我们约定每次绘图后总是马上将frame buffer中的内容同步到屏幕上.
size_t fb_write(const void *buf, size_t offset, size_t len) {
  //code:
  AM_GPU_MEMCPY_T info = {
    .dest =  offset / sizeof(uint32_t),
    .src = buf,
    .size = len / sizeof(uint32_t)
  };
  ioe_write(AM_GPU_MEMCPY, &info);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
