#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void pstree(int pid, int level) {
    int i;
    for (i = 0; i < level; ++i)
        printf("  "); // 控制縮進

    printf("%d\n", pid); // 印出當前進程的PID

    // 創建子進程
    if (fork() == 0) {
        // 子進程
        // 遞歸調用pstree函數
        pstree(pid * 2, level + 1);
    } else {
        wait(NULL); // 等待子進程結束
    }
}

int main() {
    // 獲取當前進程的PID
    int pid = getpid();
    printf("Process Tree for PID %d:\n", pid);

    // 調用pstree函數
    pstree(pid, 0);

    return 0;
}
