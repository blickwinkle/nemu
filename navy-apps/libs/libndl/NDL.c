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
static int screen_w = 0, screen_h = 0, canvas_w = 0, canvas_h = 0, canvas_y = 0, canvas_x = 0;

uint32_t NDL_GetTicks() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  int fp = open("/dev/events", O_RDONLY);

  return read(fp, buf, sizeof(char) * len);
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
     if (*w == 0){
    *w = screen_w;
  }else if(*w > screen_w){
    
  }
  if (*h == 0){
    *h = screen_h;
  }else if(*h > screen_h){
    
  }
  canvas_w = *w;
  canvas_h = *h;
  return ;
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
  if (x == y == w == h == 0) {
    x = y = 0;
    w = canvas_w;
    h = canvas_h;
  }
  fbdev = open("/dev/fb", 0);
  if (fbdev < 0) {
    fprintf(stderr, "failed to open /dev/fb\n");
    exit(1);
  }
  // printf("NDL_DrawRect: x = %d, y = %d, w = %d, h = %d\n", x, y, w, h);
  for (int i = 0; i < h; i ++) {
    lseek(fbdev, (y + i) * screen_w * sizeof(uint32_t) + x * sizeof(uint32_t), SEEK_SET);
    // printf("NDL_DrawRect: lseek = %d\n", ((y + i) * screen_w * sizeof(uint32_t) + x * sizeof(uint32_t)) / 4);
    write(fbdev, pixels + i * w, w * sizeof(uint32_t));
  }
  return ;

  int graphics = open("/dev/fb", O_RDWR);
  
  for (int i = 0; i < h; ++i){
    lseek(graphics, ((canvas_y + y + i) * screen_w + (canvas_x + x)) * sizeof(uint32_t), SEEK_SET);
    ssize_t s = write(graphics, pixels + w * i, w * sizeof(uint32_t));
  }
}

void NDL_DrawPoint(uint32_t color, int x, int y) {
  lseek(fbdev, (y * screen_w + x) * sizeof(uint32_t), SEEK_SET);
  write(fbdev, &color, sizeof(uint32_t));
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

static void read_key_value(char *str, char *key, int* value){
  char buffer[128];
  int len = 0;
  for (char* c = str; *c; ++c){
    if(*c != ' '){
      buffer[len++] = *c;
    }
  }
  buffer[len] = '\0';

  sscanf(buffer, "%[a-zA-Z]:%d", key, value);
  // printf("read_key_value\n");
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }

  char info[128], key[64];
  int value;

  //memset(info, 0, 128);
  int dispinfo = open("/proc/dispinfo", 0);
  read(dispinfo, info, sizeof(info));
  close(dispinfo);
  // printf("%s \n", info);

  /* 获取第一个子字符串 */
  char *token = strtok(info, "\n");
   
   /* 继续获取其他的子字符串 */
   while( token != NULL ) {
      
      // printf("while begin 105 %s \n", info);
      //printf("%s = %d\n", key, value);
      read_key_value(token, key, &value);

      if(strcmp(key, "WIDTH") == 0){
        screen_w = value;
      }else if(strcmp(key, "HEIGHT") == 0) {
        screen_h = value;
      }

      // printf("while middle 105 %s \n", info);
      token = strtok(NULL, "\n");
      // printf("while end 105 %s \n", info);
  }

  printf("With width = %d, height = %d.\n", screen_w, screen_h);

  return 0;
}

void NDL_Quit() {
}
