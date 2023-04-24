#include <am.h>
#include <nemu.h>
#include <unistd.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00) 
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)
#define AUDIO_MUTEX_ADDR     (AUDIO_ADDR + 0x18)

// freq, channels和samples这三个寄存器可写入相应的初始化参数
// init寄存器用于初始化, 写入后将根据设置好的freq, channels和samples来对SDL的音频子系统进行初始化
// 流缓冲区STREAM_BUF是一段MMIO空间, 用于存放来自程序的音频数据, 这些音频数据会在将来写入到SDL库中
// sbuf_size寄存器可读出流缓冲区的大小
// count寄存器可以读出当前流缓冲区已经使用的大小

// AM_AUDIO_CONFIG, AM声卡控制器信息, 可读出存在标志present以及流缓冲区的大小bufsize. 另外AM假设系统在运行过程中, 流缓冲区的大小不会发生变化.
// AM_AUDIO_CTRL, AM声卡控制寄存器, 可根据写入的freq, channels和samples对声卡进行初始化.
// AM_AUDIO_STATUS, AM声卡状态寄存器, 可读出当前流缓冲区已经使用的大小count.
// AM_AUDIO_PLAY, AM声卡播放寄存器, 可将[buf.start, buf.end)区间的内容作为音频数据写入流缓冲区. 若当前流缓冲区的空闲空间少于即将写入的音频数据, 此次写入将会一直等待, 直到有足够的空闲空间将音频数据完全写入流缓冲区才会返回.

static int freq = 0, channels = 0, samples = 0, sbuf_size = 0;

void __am_audio_init() {
  
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
  cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  freq = ctrl->freq;
  channels = ctrl->channels;
  samples = ctrl->samples;
  sbuf_size = inl(AUDIO_SBUF_SIZE_ADDR);
  outl(AUDIO_FREQ_ADDR, freq);
  outl(AUDIO_CHANNELS_ADDR, channels);
  outl(AUDIO_SAMPLES_ADDR, samples);
  outl(AUDIO_INIT_ADDR, 1);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR);
}

static int last_write = 0;

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  int count;
  while (1) {
    count = inl(AUDIO_COUNT_ADDR);
    if (count + (ctl->buf.end - ctl->buf.start) <= sbuf_size) {
      break;
    }
  }
  int i, write_count = ctl->buf.end - ctl->buf.start;
  for (i = 0; i < write_count; i++) {
    last_write += 1;
    last_write %= sbuf_size;
    outb(AUDIO_SBUF_ADDR + last_write, ((uint8_t *)(ctl->buf.start))[i]);
  }
  outl(AUDIO_COUNT_ADDR, count + write_count);
}
