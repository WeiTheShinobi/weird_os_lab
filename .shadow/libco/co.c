#include "co.h"
#include "assert.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <string.h>

typedef struct {
    ucontext_t context;
} context;

context *new_context() {
    context *ctx = malloc(sizeof(context));
    if (!ctx) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    return ctx;
}
// 保存当前上下文
void save_context(context *ctx) {
    if (getcontext(&ctx->context) == -1) {
        perror("getcontext");
        exit(EXIT_FAILURE);
    }
}

// 恢复保存的上下文
void restore_context(context *ctx) {
    if (setcontext(&ctx->context) == -1) {
        perror("setcontext");
        exit(EXIT_FAILURE);
    }
}

#define STACK_SIZE 100

enum co_status {
  CO_NEW = 1, // 新创建，还未执行过
  CO_RUNNING, // 已经执行过
  CO_WAITING, // 在 co_wait 上等待
  CO_DEAD,    // 已经结束，但还未释放资源
};

struct co {
  char *name;
  void (*func)(void *); // co_start 指定的入口地址和参数
  void *arg;

  enum co_status status;     // 协程的状态
  struct co *waiter;         // 是否有其他协程在等待当前协程
  struct context context;    // 寄存器现场
  uint8_t stack[STACK_SIZE]; // 协程的堆栈
};

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  context *cx = new_context();
  printf("create co\n");
  save_context(cx);
  printf("%s", context_to_string(cx));

  return NULL;
}

void co_wait(struct co *co) {}

void co_yield () {}
