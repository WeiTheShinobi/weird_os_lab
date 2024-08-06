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
static CoNode *current = NULL;

static void co_node_insert(struct co *coroutine) {
	CoNode *victim = (CoNode*)malloc(sizeof(CoNode));
	assert(victim);


	victim->coroutine = coroutine;
	if(co_node == NULL) {
		victim->fd = victim->bk = victim;
		co_node = victim;
	}else {
		victim->fd = co_node->fd;
		victim->bk = co_node;
		victim->fd->bk = victim->bk->fd = victim;
	}
}

static CoNode *co_node_remove() {
	CoNode *victim = NULL;

	if(co_node == NULL) { return NULL; }
	else if(co_node->bk == co_node) {
		victim = co_node;
		co_node = NULL;
	}else {
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
  jmp_buf *context;          // 寄存器现场
  uint8_t stack[STACK_SIZE]; // 协程的堆栈
};

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
  free(co_node_remove());
}

void co_yield () {}

static inline void stack_switch_call(void *sp, void *entry, void *arg) {
  asm volatile("movq %%rcx, 0(%0); movq %0, %%rsp; movq %2, %%rdi; call *%1"
               :
               : "b"((uintptr_t)sp - 16), "d"((uintptr_t)entry),
                 "a"((uintptr_t)arg));
}
/*
 * 从调用的指定函数返回，并恢复相关的寄存器。此时协程执行结束，以后再也不会执行该协程的上下文。这里需要注意的是，其和上面并不是对称的，因为调用协程给了新创建的选中协程的堆栈，则选中协程以后就在自己的堆栈上执行，永远不会返回到调用协程的堆栈。
 */
static inline void restore_return() {
  asm volatile("movq 0(%%rsp), %%rcx" : :);
}
