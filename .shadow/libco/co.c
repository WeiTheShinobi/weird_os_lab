#include "co.h"
#include "assert.h"
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>



typedef struct CONODE {
  struct co *coroutine;

  struct CONODE *fd, *bk;
} CoNode;

static CoNode *co_node = NULL;
struct co *current;

static void co_node_insert(struct co *coroutine) {
  CoNode *victim = (CoNode *)malloc(sizeof(CoNode));
  assert(victim);

  victim->coroutine = coroutine;
  if (co_node == NULL) {
    victim->fd = victim->bk = victim;
    co_node = victim;
  } else {
    victim->fd = co_node->fd;
    victim->bk = co_node;
    victim->fd->bk = victim->bk->fd = victim;
  }
}

static CoNode *co_node_remove() {
  CoNode *victim = NULL;

  if (co_node == NULL) {
    return NULL;
  } else if (co_node->bk == co_node) {
    victim = co_node;
    co_node = NULL;
  } else {
    victim = co_node;

    co_node = co_node->bk;
    co_node->fd = victim->fd;
    co_node->fd->bk = co_node;
  }

  return victim;
}

#define STACK_SIZE 1024 * 12

enum co_status {
  CO_NEW = 1, // 新创建，还未执行过
  CO_RUNNING, // 已经执行过
  CO_WAITING, // 在 co_wait 上等待
  CO_DEAD,    // 已经结束，但还未释放资源
};

struct co {
  const char *name;
  void (*func)(void *); // co_start 指定的入口地址和参数
  void *arg;

  enum co_status status;     // 协程的状态
  struct co *waiter;         // 是否有其他协程在等待当前协程
  jmp_buf context;           // 寄存器现场
  uint8_t stack[STACK_SIZE]; // 协程的堆栈
};


static inline void stack_switch_call(void *sp, void *entry, void *arg) {
  asm volatile("movq %%rcx, 0(%0); movq %0, %%rsp; movq %2, %%rdi; call *%1"
               :
               : "b"((uintptr_t)sp - 16), "d"((uintptr_t)entry),
                 "a"((uintptr_t)arg));
}

static inline void restore_return() {
  asm volatile("movq 0(%%rsp), %%rcx" : :);
}

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  struct co *new_co = malloc(sizeof(struct co));
  assert(new_co != NULL);

  new_co->name = name;
  new_co->func = func;
  new_co->arg = arg;
  new_co->status = CO_NEW;
  new_co->waiter = NULL;

  co_node_insert(new_co);

  return new_co;
}

void co_wait(struct co *co) {
  if (co->status != CO_DEAD) {
    current->status = CO_WAITING;
    co->waiter = current;
    co_yield ();
  }

  free(co);
  free(co_node_remove());
}

#define __LONG_JUMP_STATUS (1)

void co_yield () {
  int status = setjmp(current->context);
  if (status == 0) {
    co_node = co_node->bk;
    while (!((current = co_node->coroutine)->status == CO_NEW ||
             current->status == CO_RUNNING)) {
      co_node = co_node->bk;
    }
    assert(current);
    printf("co_yield: %s\n", current->name);
    if (current->status == CO_RUNNING) {
      longjmp(current->context, __LONG_JUMP_STATUS);
    }

  } else {
    stack_switch_call(current->stack + sizeof(current->stack), current->func, current->arg);
    restore_return();
  }
}


static __attribute__((constructor)) void co_constructor(void) {
  current = co_start("main", NULL, NULL);
  current->status = CO_RUNNING;
}

static __attribute__((destructor)) void co_destructor(void) {
  if (co_node == NULL) {
    return;
  }

  while (co_node) {
    current = co_node->coroutine;
    free(current);
    free(co_node_remove());
  }
}
