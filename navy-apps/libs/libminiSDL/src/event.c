#include <NDL.h>
#include <SDL.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  //将NDL中提供的事件封装成SDL事件
  //并将SDL事件放入event中
  //返回0表示成功, 返回-1表示失败
  char buf[64];
  int ret = NDL_PollEvent(buf, sizeof(buf));
  if (ret <= 0) return -1;
  int key = 0, is_down = 0;
  sscanf(buf, "%d %d", &is_down, &key);
  if (is_down) {
    event->type = SDL_KEYDOWN;
  } else {
    event->type = SDL_KEYUP;
  }
  printf("SDL_WaitEvent: key = %d, is_down : %d\n", key, is_down);
  event->key.keysym.sym = key;
  return 0;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
