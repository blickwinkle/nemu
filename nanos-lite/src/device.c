
#include <common.h>

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
  AM_GPU_CONFIG_T info = io_read(AM_GPU_CONFIG);
  // 2. 将信息写入buf
  return snprintf(buf, len, "WIDTH : %d\nHEIGHT:%d\n", info.width, info.height);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
