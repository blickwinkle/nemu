#include "sys/time.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0, canvas_w = 0, canvas_h = 0;

uint32_t NDL_GetTicks() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  int fd = open("/dev/events", 0);
  if (fd < 0) return 0;
  int ret = read(fd, buf, len);
  close(fd);
  return ret;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  } else {
    // 打开一张(*w) X (*h)的画布
    // 如果*w和*h均为0, 则将系统全屏幕作为画布, 并将*w和*h分别设为系统屏幕的大小

    int fd = open("/proc/dispinfo", 0);
    if (fd < 0) {
      fprintf(stderr, "failed to open /proc/dispinfo\n");
      exit(1);
    }
    char buf[64];
    int nread = read(fd, buf, sizeof(buf) - 1);
    if (nread <= 0) {
      fprintf(stderr, "failed to read /proc/dispinfo\n");
      exit(1);
    }
    buf[nread] = '\0';
    // fprintf(stdout, "111 NDL_OpenCanvas: screen_w = %d, screen_h = %d, canvas_w = %d, canvas_h = %d\n", screen_w, screen_h, canvas_w, canvas_h);
    sscanf(buf, "%d %d", &screen_w, &screen_h);
    close(fd);

    fbdev = open("/dev/fb", 0);
    if (fbdev < 0) {
      fprintf(stderr, "failed to open /dev/fb\n");
      exit(1);
    }
    if (*w == 0 || *h == 0) {
      *w = screen_w;
      *h = screen_h;
    }
    canvas_h = *h;
    canvas_w = *w;
    printf("NDL_OpenCanvas: screen_w = %d, screen_h = %d, canvas_w = %d, canvas_h = %d\n", screen_w, screen_h, canvas_w, canvas_h);
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  // printf("NDL_DrawRect: x = %d, y = %d, w = %d, h = %d\n", x, y, w, h);
  for (int i = 0; i < h; i ++) {
    lseek(fbdev, (y + i) * screen_w * sizeof(uint32_t) + x * sizeof(uint32_t), SEEK_SET);
    // printf("NDL_DrawRect: lseek = %d\n", ((y + i) * screen_w * sizeof(uint32_t) + x * sizeof(uint32_t)) / 4);
    write(fbdev, pixels + i * w, w * sizeof(uint32_t));
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  return 0;
}

void NDL_Quit() {
}
