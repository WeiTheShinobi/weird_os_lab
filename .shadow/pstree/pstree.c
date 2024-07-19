#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#define PROC_DIR "/proc"

typedef struct Process {
  int pid;
  struct Process **child_arr;
  int child_arr_cap;
  int child_arr_len;
} Process;

int is_int(const char *str) {
  while (*str) {
    if (!isdigit(*str)) {
      return 0;
    }
    str++;
  }
  return 1;
}

Process *new_process(int pid) {
  Process *proc = malloc(sizeof(Process));
  proc->pid = pid;
  int cap = 8;
  proc->child_arr = malloc(sizeof(Process *) * cap);
  proc->child_arr_cap = 8;
  proc->child_arr_len = 0;

  return proc;
}

void process_printf(Process *proc) {
  printf("pid: %d\n", proc->pid);
  for (int i = 0; i < proc->child_arr_len; i++) {
    process_printf(proc->child_arr[i]);
  }
}

void add_child_proc(Process *proc, Process *child) {
  if (proc->child_arr_len == proc->child_arr_cap) {
    proc->child_arr_cap *= 2;
    proc->child_arr = (Process **)realloc(
        proc->child_arr, sizeof(Process *) * proc->child_arr_cap);
  }

  proc->child_arr_len++;
  proc->child_arr[proc->child_arr_len] = child;
}

size_t parse_ppid(int pid) {
  char path[256];
  snprintf(path, sizeof(path), "/proc/%d/status", pid);
  FILE *file = fopen(path, "r");
  if (!file) {
    perror("fopen error");
    return -1;
  }

  int ppid;
  fscanf(file, "%*d %*s %*c %d", &ppid);
  fclose(file);

  printf("%d", ppid);
  return ppid;
}

int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);

  DIR *proc_dir = opendir(PROC_DIR);
  if (!proc_dir) {
    perror("opendir");
    return EXIT_FAILURE;
  }

  Process *proc_arr[32678] = {NULL};

  struct dirent *entry;
  while ((entry = readdir(proc_dir)) != NULL) {
    if (entry->d_type == DT_DIR && is_int(entry->d_name)) {
      int pid = atoi(entry->d_name);
      size_t ppid = parse_ppid(pid);
      Process *proc = new_process(pid);
      // add_child_proc(proc_arr[ppid], proc);
      proc_arr[pid] = proc;
    }
  }

  closedir(proc_dir);
  process_printf(proc_arr[1]);

  return 0;
}
