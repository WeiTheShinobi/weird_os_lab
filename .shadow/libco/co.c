#include "co.h"
#include "assert.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#
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

context* new_context() {
  context *cx = (context *)calloc(1, sizeof(context));
  assert(cx != NULL);
  cx->rax = 0;
  cx->rbx = 0;
  cx->rcx = 0;
  cx->rdx = 0;
  cx->rsi = 0;
  cx->rdi = 0;
  cx->rbp = 0;
  cx->rsp = 0;
  cx->r8 = 0;
  cx->r9 = 0;
  cx->r10 = 0;
  cx->r11 = 0;
  cx->r12 = 0;
  cx->r13 = 0;
  cx->r14 = 0;
  cx->r15 = 0;
  cx->rip = 0;
  cx->eflags = 0;
  cx->cs = 0;
  cx->ss = 0;
  cx->ds = 0;
  cx->es = 0;
  cx->fs = 0;
  cx->gs = 0;
  cx->fs_base = 0;
  cx->gs_base = 0;
  return cx;
}

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
               "1: lea 1b(%%rip), %16\n\t"
               "pushfq\n\t"
               "pop %17\n\t"
               "mov %%cs, %18\n\t"
               "mov %%ss, %19\n\t"
               "mov %%ds, %20\n\t"
               "mov %%es, %21\n\t"
               "mov %%fs, %22\n\t"
               "mov %%gs, %23\n\t"
               "mov %%fs:0, %24\n\t"
               "mov %%gs:0, %25\n\t"
               : "=r"(cx->rax), "=m"(cx->rbx), "=m"(cx->rcx), "=m"(cx->rdx),
                 "=m"(cx->rsi), "=m"(cx->rdi), "=m"(cx->rbp), "=m"(cx->rsp),
                 "=m"(cx->r8), "=m"(cx->r9), "=m"(cx->r10), "=m"(cx->r11),
                 "=m"(cx->r12), "=m"(cx->r13), "=m"(cx->r14), "=m"(cx->r15),
                 "=a"(cx->rip), "=r"(cx->eflags), "=m"(cx->cs), "=m"(cx->ss),
                 "=m"(cx->ds), "=m"(cx->es), "=m"(cx->fs), "=m"(cx->gs),
                 "=r"(cx->fs_base), "=r"(cx->gs_base));
}

char *context_to_string(context *cx) {
  size_t buffer_size = 1024;
  char *buffer = (char *)calloc(1024, sizeof(char));
  assert(buffer != NULL);

  snprintf(buffer, buffer_size,
           "rax: 0x%016llu\n"
           "rbx: 0x%016llu\n"
           "rcx: 0x%016llu\n"
           "rdx: 0x%016llu\n"
           "rsi: 0x%016llu\n"
           "rdi: 0x%016llu\n"
           "rbp: 0x%016llu\n"
           "rsp: 0x%016llu\n"
           "r8:  0x%016llu\n"
           "r9:  0x%016llu\n"
           "r10: 0x%016llu\n"
           "r11: 0x%016llu\n"
           "r12: 0x%016llu\n"
           "r13: 0x%016llu\n"
           "r14: 0x%016llu\n"
           "r15: 0x%016llu\n"
           "rip: 0x%016llu\n"
           "eflags: 0x%016llu\n"
           "cs: 0x%016llu\n"
           "ss: 0x%016llu\n"
           "ds: 0x%016llu\n"
           "es: 0x%016llu\n"
           "fs: 0x%016llu\n"
           "gs: 0x%016llu\n"
           "fs_base: 0x%016llu\n"
           "gs_base: 0x%016llu\n",
           cx->rax, cx->rbx, cx->rcx, cx->rdx, cx->rsi, cx->rdi, cx->rbp,
           cx->rsp, cx->r8, cx->r9, cx->r10, cx->r11, cx->r12, cx->r13, cx->r14,
           cx->r15, cx->rip, cx->eflags, cx->cs, cx->ss, cx->ds, cx->es, cx->fs,
           cx->gs, cx->fs_base, cx->gs_base);

  return buffer;
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
  context_save(cx);
  printf("%s", context_to_string(cx));

  return NULL;
}

void co_wait(struct co *co) {}

void co_yield () {}
