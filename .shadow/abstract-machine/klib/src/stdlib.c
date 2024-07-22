#include <am.h>
#include <klib-macros.h>
#include <klib.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next / 65536) % 32768;
}

void srand(unsigned int seed) { next = seed; }

int abs(int x) { return (x < 0 ? -x : x); }

int atoi(const char *nptr) {
  int x = 0;
  while (*nptr == ' ') {
    nptr++;
  }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr++;
  }
  return x;
}

void itoa(int n, void* ss) {
  char *s = ss;
  int i, j, sign;
  if ((sign = n) < 0) // 記錄符號
    n = -n;           // 使n成為正數
  i = 0;
  do {
    s[i++] = n % 10 +'0'; // 取下一個數字
  } while ((n /= 10) > 0); // 刪除該數字
  if (sign < 0)
    s[i++] ='-';
  s[i] ='\0';
  for (j = i; j >= 0; j--) // 生成的數字是逆序的，所以要逆序輸出
    printf("%c", s[j]);
}

void *malloc(size_t size) {
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  panic("Not implemented");
#endif
  return NULL;
}

void free(void *ptr) {}

#endif
