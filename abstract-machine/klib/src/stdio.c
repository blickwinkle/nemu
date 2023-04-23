#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)



int vsprintf(char *out, const char *fmt, va_list ap) {
  const char *p = out;
  while (*fmt != '\0') {
    if (*fmt == '%') {
      fmt++;
      switch (*fmt) {
        case 'd': {
          int x = va_arg(ap, int);
          char buf[32];
          int len = 0;
          if (x < 0) {
            *out = '-';
            out++;
            x = -x;
          }
          do {
            buf[len++] = x % 10 + '0';
            x /= 10;
          } while (x > 0);
          for (int i = len - 1; i >= 0; i--) {
            *out = buf[i];
            out++;
          }
          break;
        }
        case 's': {
          char *s = va_arg(ap, char *);
          while (*s != '\0') {
            *out = *s;
            out++;
            s++;
          }
          break;
        }
        case 'c': {
          char c = va_arg(ap, int);
          *out = c;
          out++;
          break;
        }
        case 'p': {
          void *p = va_arg(ap, void *);
          char buf[32];
          int len = 0;
          do {
            int x = (uintptr_t)p % 16;
            if (x < 10) {
              buf[len++] = x + '0';
            } else {
              buf[len++] = x - 10 + 'a';
            }
            p = (void *)((uintptr_t)p / 16);
          } while ((uintptr_t)p > 0);
          *out = '0';
          out++;
          *out = 'x';
          out++;
          for (int i = len - 1; i >= 0; i--) {
            *out = buf[i];
            out++;
          }
          break;
        }
        case 'x': {
          int x = va_arg(ap, int);
          char buf[32];
          int len = 0;
          do {
            int y = x % 16;
            if (y < 10) {
              buf[len++] = y + '0';
            } else {
              buf[len++] = y - 10 + 'a';
            }
            x /= 16;
          } while (x > 0);
          for (int i = len - 1; i >= 0; i--) {
            *out = buf[i];
            out++;
          }
          break;
        }
        case '%': {
          *out = '%';
          out++;
          break;
        }
        //处理类似%02d这种格式，如果输出的整型数不足两位，左侧用0补齐。需要能 处理任意位数的情况。
        case '0': {
          fmt++;
          uint8_t width = *fmt - '0';
          fmt++;
          char c = *(fmt);

          if (c != 'd') {
            printf("Unknown format specifier : %c\n", *fmt);
            panic("Unknown format specifier");
          }
          int x = va_arg(ap, int);
          char buf[32] = {0};
          int len = 0;
          bool is_neg = false;
          if (x < 0) {
            is_neg = true;
            x = -x;
          }
          do {
            buf[len++] = x % 10;
            x /= 10;
          } while (x > 0);
          if (is_neg) {
            buf[len++] = '-';
          }
          // 输出补全的0
          while (len < width) {
            buf[len++] = '0';
          }
          for (int i = len - 1; i >= 0; i--) {
            *out = buf[i];
            out++;
          }
          printf(buf);
          break;
        }
        default: {
          printf("Unknown format specifier : %c\n", *fmt);
          panic("Unknown format specifier");
        }
      }
    } else {
      *out = *fmt;
      out++;
    }
    fmt++;
  }
  *out = '\0';
  return out - p;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = vsprintf(out, fmt, ap);
  va_end(ap);
  return ret;
}

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char buf[1024];
  int ret = vsprintf(buf, fmt, ap);
  va_end(ap);
  putstr(buf);
  return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
