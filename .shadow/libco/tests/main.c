#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "co-test.h"

int g_count = 0;

static void add_count() {
    g_count++;
}

static int get_count() {
    return g_count;
}

static void work_loop(void *arg) {
    const char *s = (const char*)arg;
    for (int i = 0; i < 100; ++i) {
        printf("%s%d  ", s, get_count());
        add_count();
        co_yield();
    }
}

static void work(void *arg) {
    work_loop(arg);
}

static void test_1() {

    struct co *thd1 = co_start("thread-1", work, "X");
    struct co *thd2 = co_start("thread-2", work, "Y");

    co_wait(thd1);
    co_wait(thd2);

//    printf("\n");
}

// -----------------------------------------------

static int g_running = 1;

static void do_produce(Queue *queue) {
    assert(!q_is_full(queue));
    Item *item = (Item*)malloc(sizeof(Item));
    if (!item) {
        fprintf(stderr, "New item failure\n");
        return;
    }
    item->data = (char*)malloc(10);
    if (!item->data) {
        fprintf(stderr, "New data failure\n");
        free(item);
        return;
    }
    memset(item->data, 0, 10);
    sprintf(item->data, "libco-%d", g_count++);
    q_push(queue, item);
}

static void producer(void *arg) {
    Queue *queue = (Queue*)arg;
    for (int i = 0; i < 100; ) {
        if (!q_is_full(queue)) {
            // co_yield();
            do_produce(queue);
            i += 1;
        }
        co_yield();
    }
}

static void do_consume(Queue *queue) {
    assert(!q_is_empty(queue));

    Item *item = q_pop(queue);
    if (item) {
        printf("%s  ", (char *)item->data);
        free(item->data);
        free(item);
    }
}

static void consumer(void *arg) {
    Queue *queue = (Queue*)arg;
    while (g_running) {
        if (!q_is_empty(queue)) {
            do_consume(queue);
        }
        co_yield();
    }
}

static void test_2() {

    Queue *queue = q_new();

    struct co *thd1 = co_start("producer-1", producer, queue);
    struct co *thd2 = co_start("producer-2", producer, queue);
    struct co *thd3 = co_start("consumer-1", consumer, queue);
    struct co *thd4 = co_start("consumer-2", consumer, queue);

    co_wait(thd1);
    co_wait(thd2);

    g_running = 0;

    co_wait(thd3);
    co_wait(thd4);

    while (!q_is_empty(queue)) {
        do_consume(queue);
    }

    q_free(queue);
}

#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>  // For user_regs_struct
#include <unistd.h>
#include <errno.h>

void print_registers(struct user_regs_struct *regs) {
    printf("RAX: 0x%llx\n", regs->rax);
    printf("RBX: 0x%llx\n", regs->rbx);
    printf("RCX: 0x%llx\n", regs->rcx);
    printf("RDX: 0x%llx\n", regs->rdx);
    printf("RSI: 0x%llx\n", regs->rsi);
    printf("RDI: 0x%llx\n", regs->rdi);
    printf("RBP: 0x%llx\n", regs->rbp);
    printf("RSP: 0x%llx\n", regs->rsp);
    printf("R8:  0x%llx\n", regs->r8);
    printf("R9:  0x%llx\n", regs->r9);
    printf("R10: 0x%llx\n", regs->r10);
    printf("R11: 0x%llx\n", regs->r11);
    printf("R12: 0x%llx\n", regs->r12);
    printf("R13: 0x%llx\n", regs->r13);
    printf("R14: 0x%llx\n", regs->r14);
    printf("R15: 0x%llx\n", regs->r15);
    printf("RIP: 0x%llx\n", regs->rip);
    printf("EFLAGS: 0x%llx\n", regs->eflags);
    printf("CS: 0x%llx\n", regs->cs);
    printf("SS: 0x%llx\n", regs->ss);
    printf("DS: 0x%llx\n", regs->ds);
    printf("ES: 0x%llx\n", regs->es);
    printf("FS: 0x%llx\n", regs->fs);
    printf("GS: 0x%llx\n", regs->gs);
    printf("FS_BASE: 0x%llx\n", regs->fs_base);
    printf("GS_BASE: 0x%llx\n", regs->gs_base);
}
int main() {
    // setbuf(stdout, NULL);

    // printf("Test #1. Expect: (X|Y){0, 1, 2, ..., 199}\n");
    // test_1();

    // printf("\n\nTest #2. Expect: (libco-){200, 201, 202, ..., 399}\n");
    // test_2();

    // printf("\n\n");


    pid_t pid = getpid();

    // Attach to the process
    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
        perror("ptrace");
        return 1;
    }

    // Wait for the process to stop
    waitpid(pid, NULL, 0);

    // Get the registers
    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) == -1) {
        perror("ptrace");
        return 1;
    }

    // Print the registers
    print_registers(&regs);

    // Detach from the process
    if (ptrace(PTRACE_DETACH, pid, NULL, NULL) == -1) {
        perror("ptrace");
        return 1;
    }

    return 0;
    return 0;
}
