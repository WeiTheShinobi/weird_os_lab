#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#define PROC_DIR "/proc"

typedef struct Process {
  int pid;
  struct Process *child;
} Process;

int is_int(const char *str) {
  while (*str) {
    if (!isdigit(*str)) {
      return 1;
    }
    str++;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);
  printf("%s\n", PROC_DIR);

  DIR *proc = opendir(PROC_DIR);
  if (!proc) {
    perror("opendir");
    return EXIT_FAILURE;
  }

  struct dirent *entry;
  while ((entry = readdir(proc)) != NULL) {
    if (entry->d_type == DT_DIR) {
      if (is_int(entry->d_name)) {
        printf("%s - %d - %d\n", entry->d_name, entry->d_type,
               is_int(entry->d_name));
      }
      // int pid = atoi(entry->d_name);
      // int ppid;
      // char name[256];
      // get_process_info(pid, &ppid, name, sizeof(name));
      // Process *proc = create_process(pid, ppid, name);
      // processes[pid] = proc;
      // if (ppid >= 0 && processes[ppid]) {
      //   add_child_process(processes[ppid], proc);
      // } else {
      //   add_child_process(root, proc);
      // }
    }
  }

  return 0;
}
