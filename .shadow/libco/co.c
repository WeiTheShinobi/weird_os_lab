#include "co.h"
#include "assert.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define STACK_SIZE 100

enum co_status {
  CO_NEW = 1, // 新创建，还未执行过
  CO_RUNNING, // 已经执行过
  CO_WAITING, // 在 co_wait 上等待
  CO_DEAD,    // 已经结束，但还未释放资源
};

typedef struct context {
  uint64_t rax;
  uint64_t rbx;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rsi;
  uint64_t rdi;
  uint64_t rbp;
  uint64_t rsp;
  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
  uint64_t rip;
  uint64_t eflags;
  uint64_t cs;
  uint64_t ss;
  uint64_t ds;
  uint64_t es;
  uint64_t fs;
  uint64_t gs;
  uint64_t fs_base;
  uint64_t gs_base;
} context;

void context_save(context *cx) {
  asm volatile("mov %%rax, %0\n\t"
               "mov %%rbx, %1\n\t"
               "mov %%rcx, %2\n\t"
               "mov %%rdx, %3\n\t"
               "mov %%rsi, %4\n\t"
               "mov %%rdi, %5\n\t"
               "mov %%rbp, %6\n\t"
               "mov %%rsp, %7\n\t"
               "mov %%r8, %8\n\t"
               "mov %%r9, %9\n\t"
               "mov %%r10, %10\n\t"
               "mov %%r11, %11\n\t"
               "mov %%r12, %12\n\t"
               "mov %%r13, %13\n\t"
               "mov %%r14, %14\n\t"
               "mov %%r15, %15\n\t"
               : "=m"(cx->rax), "=m"(cx->rbx), "=m"(cx->rcx), "=m"(cx->rdx),
                 "=m"(cx->rsi), "=m"(cx->rdi), "=m"(cx->rbp), "=m"(cx->rsp),
                 "=m"(cx->r8), "=m"(cx->r9), "=m"(cx->r10), "=m"(cx->r11),
                 "=m"(cx->r12), "=m"(cx->r13), "=m"(cx->r14), "=m"(cx->r15));
}

char *context_to_string(context *co) {
  size_t buffer_size = 1024;
  char *buffer = (char *)calloc(1024, sizeof(char));
  assert(buffer != NULL);

  snprintf(buffer, buffer_size,
           "rax: 0x%016lu\n"
           "rbx: 0x%016lu\n"
           "rcx: 0x%016lu\n"
           "rdx: 0x%016lu\n"
           "rsi: 0x%016lu\n"
           "rdi: 0x%016lu\n"
           "rbp: 0x%016lu\n"
           "rsp: 0x%016lu\n"
           "r8:  0x%016lu\n"
           "r9:  0x%016lu\n"
           "r10: 0x%016lu\n"
           "r11: 0x%016lu\n"
           "r12: 0x%016lu\n"
           "r13: 0x%016lu\n"
           "r14: 0x%016lu\n"
           "r15: 0x%016lu\n"
           "rip: 0x%016lu\n"
           "eflags: 0x%016lu\n"
           "cs: 0x%016lu\n"
           "ss: 0x%016lu\n"
           "ds: 0x%016lu\n"
           "es: 0x%016lu\n"
           "fs: 0x%016lu\n"
           "gs: 0x%016lu\n"
           "fs_base: 0x%016lu\n"
           "gs_base: 0x%016lu\n",
           co->rax, co->rbx, co->rcx, co->rdx, co->rsi, co->rdi, co->rbp,
           co->rsp, co->r8, co->r9, co->r10, co->r11, co->r12, co->r13, co->r14,
           co->r15, co->rip, co->eflags, co->cs, co->ss, co->ds, co->es, co->fs,
           co->gs, co->fs_base, co->gs_base);

  return buffer;
}

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
  context *cx = (context *)calloc(1, sizeof(context));
  context_save(cx);

  return NULL;
}

void co_wait(struct co *co) {}

void co_yield () {}
