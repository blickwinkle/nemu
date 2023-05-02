#include "riscv/riscv.h"
#include <am.h>
#include <nemu.h>
#include <stdint.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)


static int width = 0, height = 0;

void __am_gpu_init() {
  // int i;
  width = inl(VGACTL_ADDR) >> 16; // TODO: get the correct width
  height = inl(VGACTL_ADDR) & 0xffff; // TODO: get the correct height
  // uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  // for (i = 0; i < width * height; i++)
  //   fb[i] = i;
  // outl(SYNC_ADDR, 1);
}

void  __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  width = inl(VGACTL_ADDR) >> 16; // TODO: get the correct width
  height = inl(VGACTL_ADDR) & 0xffff; // TODO: get the correct height
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = width, .height = height,
    .vmemsz = 0
  };
}

void __am_gpu_memcpy(AM_GPU_MEMCPY_T *req) {
  // AM_GPU_MEMCPY, AM帧缓冲控制器, 可写入绘图信息, 将pixels中的w*h个像素复制到屏幕(x, y)坐标处. 图像像素按行优先方式存储在pixels中, 每个像素用32位整数以00RRGGBB的方式描述颜色.
  // uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  // fb += req->dest;
  // for (int i = 0; i < req->size; i++) {
  //   fb[i] = ((uint32_t *)(req->src))[i];
  // }
  // outl(SYNC_ADDR, 1);

  uint32_t *src = (uint32_t *)req->src, *dst = (uint32_t *)(FB_ADDR + req->dest);
  for (int i = 0; i < req->size >> 2; i++, src++, dst++){
    *dst = *src;
  }
  char *c_src = (char *)src, *c_dst = (char *)dst;
  for (int i = 0; i < (req->size & 3); i++){
    c_dst[i] = c_src[i];
  }
  outl(SYNC_ADDR, 1);
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  // AM_GPU_FBDRAW, AM帧缓冲控制器, 可写入绘图信息, 向屏幕(x, y)坐标处绘制w*h的矩形图像. 图像像素按行优先方式存储在pixels中, 每个像素用32位整数以00RRGGBB的方式描述颜色. 若sync为true, 则马上将帧缓冲中的内容同步到屏幕上.
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  int i, j;
  for (i = 0; i < h; i++) {
    for (j = 0; j < w; j++) {
      fb[(y + i) * width + x + j] = ((uint32_t *)(ctl->pixels))[i * w + j];
    }
  }

  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
  
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
