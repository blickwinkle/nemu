#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define BUF_SIZE 1024


int vsprintf(char *out, const char *fmt, va_list ap) {
  const char *old_fmt = fmt;
  const char *p = out;
  while (*fmt != '\0') {
    if (*fmt == '%') {
      fmt++;
      switch (*fmt) {
        case 'd': {
          int x = va_arg(ap, int);
          char buf[BUF_SIZE];
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
          char buf[BUF_SIZE];
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
          uint32_t x = va_arg(ap, uint32_t);
          char buf[BUF_SIZE];
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
          while (c >= '0' && c <= '9') {
            width = width * 10 + c - '0';
            fmt++;
            c = *(fmt);
          }
          if (c == 'd') {
            int x = va_arg(ap, int);
            char buf[BUF_SIZE] = {0};
            int len = 0;
            bool is_neg = false;
            if (x < 0) {
              is_neg = true;
              x = -x;
            }
            do {
              buf[len++] = x % 10 + '0';
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
            break;
          } else if (c == 'x') {
            int x = va_arg(ap, int);
            char buf[BUF_SIZE] = {0};
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
            // 输出补全的0
            while (len < width) {
              buf[len++] = '0';
            }
            for (int i = len - 1; i >= 0; i--) {
              *out = buf[i];
              out++;
            }
            break;
          } else {
            printf("Unknown format specifier : %c\n", *fmt);
            printf("%s", old_fmt);
            panic("Unknown format specifier");
          }
          
        }
        case 'u': {
          unsigned int x = va_arg(ap, unsigned int);
          char buf[BUF_SIZE];
          int len = 0;
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
        default: {
          //处理类似%3d这种格式：
          if (*fmt >= '1' && *fmt <= '9') {
            uint8_t width = *fmt - '0';
            fmt++;
            char c = *(fmt);
            while (c >= '0' && c <= '9') {
              width = width * 10 + c - '0';
              fmt++;
              c = *(fmt);
            }
            if (c == 'd') {
              int x = va_arg(ap, int);
              char buf[BUF_SIZE] = {0};
              int len = 0;
              bool is_neg = false;
              if (x < 0) {
                is_neg = true;
                x = -x;
              }
              do {
                buf[len++] = x % 10 + '0';
                x /= 10;
              } while (x > 0);
              if (is_neg) {
                buf[len++] = '-';
              }
              // 输出补全的0
              while (len < width) {
                buf[len++] = ' ';
              }
              for (int i = len - 1; i >= 0; i--) {
                *out = buf[i];
                out++;
              }
              break;
            } else if (c == 'x') {
              uint32_t x = va_arg(ap, uint32_t);
              char buf[BUF_SIZE] = {0};
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
              // 输出补全的0
              while (len < width) {
                buf[len++] = ' ';
              }
              for (int i = len - 1; i >= 0; i--) {
                *out = buf[i];
                out++;
              }
              break;
            } else {
              printf("Unknown format specifier : %c\n", *fmt);
              printf("%s", old_fmt);
              panic("Unknown format specifier");
            }
          }
          printf("Unknown format specifier : %c\n", *fmt);
          printf("%s", old_fmt);
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
  char buf[1024 * 16];
  int ret = vsprintf(buf, fmt, ap);
  va_end(ap);
  putstr(buf);
  return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  // code:
  va_list ap;
  va_start(ap, fmt);
  int ret = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return ret;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  // code:
  #define increase_and_check_length \
    do { \
      out++; \
      if ((out - p) >= n) { \
        return out - p; \
      } \
    } while (0)

  const char *old_fmt = fmt;
  const char *p = out;
  while (*fmt != '\0') {
    if (*fmt == '%') {
      fmt++;
      switch (*fmt) {
        case 'd': {
          int x = va_arg(ap, int);
          char buf[BUF_SIZE];
          int len = 0;
          if (x < 0) {
            *out = '-';
            increase_and_check_length;
            x = -x;
          }
          do {
            buf[len++] = x % 10 + '0';
            x /= 10;
          } while (x > 0);
          for (int i = len - 1; i >= 0; i--) {
            *out = buf[i];
            increase_and_check_length;
          }
          break;
        }
        case 's': {
          char *s = va_arg(ap, char *);
          while (*s != '\0') {
            *out = *s;
            increase_and_check_length;
            s++;
          }
          break;
        }
        case 'c': {
          char c = va_arg(ap, int);
          *out = c;
          increase_and_check_length;
          break;
        }
        case 'p': {
          void *_p = va_arg(ap, void *);
          char buf[BUF_SIZE];
          int len = 0;
          do {
            int x = (uintptr_t)_p % 16;
            if (x < 10) {
              buf[len++] = x + '0';
            } else {
              buf[len++] = x - 10 + 'a';
            }
            _p = (void *)((uintptr_t)_p / 16);
          } while ((uintptr_t)_p > 0);
          *out = '0';
          increase_and_check_length;
          *out = 'x';
          increase_and_check_length;
          for (int i = len - 1; i >= 0; i--) {
            *out = buf[i];
            increase_and_check_length;
          }
          break;
        }
        case 'x': {
          uint32_t x = va_arg(ap, uint32_t);
          char buf[BUF_SIZE];
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
            increase_and_check_length;
          }
          break;
        }
        case '%': {
          *out = '%';
          increase_and_check_length;
          break;
        }
        //处理类似%02d这种格式，如果输出的整型数不足两位，左侧用0补齐。需要能 处理任意位数的情况。
        case '0': {
          fmt++;
          uint8_t width = *fmt - '0';
          fmt++;
          char c = *(fmt);
          while (c >= '0' && c <= '9') {
            width = width * 10 + c - '0';
            fmt++;
            c = *(fmt);
          }
          if (c == 'd') {
            int x = va_arg(ap, int);
            char buf[BUF_SIZE] = {0};
            int len = 0;
            bool is_neg = false;
            if (x < 0) {
              is_neg = true;
              x = -x;
            }
            do {
              buf[len++] = x % 10 + '0';
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
              increase_and_check_length;
            }
            break;
          } else if (c == 'x') {
            int x = va_arg(ap, int);
            char buf[BUF_SIZE] = {0};
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
            // 输出补全的0
            while (len < width) {
              buf[len++] = '0';
            }
            for (int i = len - 1; i >= 0; i--) {
              *out = buf[i];
              increase_and_check_length;
            }
            break;
          } else {
            printf("Unknown format specifier : %c\n", *fmt);
            printf("%s", old_fmt);
            panic("Unknown format specifier");
          }
          
        }
        case 'u': {
          unsigned int x = va_arg(ap, unsigned int);
          char buf[BUF_SIZE];
          int len = 0;
          do {
            buf[len++] = x % 10 + '0';
            x /= 10;
          } while (x > 0);
          for (int i = len - 1; i >= 0; i--) {
            *out = buf[i];
            increase_and_check_length;
          }
          break;
        }
        default: {
          //处理类似%3d这种格式：
          if (*fmt >= '1' && *fmt <= '9') {
            uint8_t width = *fmt - '0';
            fmt++;
            char c = *(fmt);
            while (c >= '0' && c <= '9') {
              width = width * 10 + c - '0';
              fmt++;
              c = *(fmt);
            }
            if (c == 'd') {
              int x = va_arg(ap, int);
              char buf[BUF_SIZE] = {0};
              int len = 0;
              bool is_neg = false;
              if (x < 0) {
                is_neg = true;
                x = -x;
              }
              do {
                buf[len++] = x % 10 + '0';
                x /= 10;
              } while (x > 0);
              if (is_neg) {
                buf[len++] = '-';
              }
              // 输出补全的0
              while (len < width) {
                buf[len++] = ' ';
              }
              for (int i = len - 1; i >= 0; i--) {
                *out = buf[i];
                increase_and_check_length;
              }
              break;
            } else if (c == 'x') {
              uint32_t x = va_arg(ap, uint32_t);
              char buf[BUF_SIZE] = {0};
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
              // 输出补全的0
              while (len < width) {
                buf[len++] = ' ';
              }
              for (int i = len - 1; i >= 0; i--) {
                *out = buf[i];
                increase_and_check_length;
              }
              break;
            } else {
              printf("Unknown format specifier : %c\n", *fmt);
              printf("%s", old_fmt);
              panic("Unknown format specifier");
            }
          }
          printf("Unknown format specifier : %c\n", *fmt);
          printf("%s", old_fmt);
          panic("Unknown format specifier");
        }
      }
    } else {
      *out = *fmt;
      increase_and_check_length;
    }
    fmt++;
  }
  *out = '\0';
  return out - p;
}

#endif
