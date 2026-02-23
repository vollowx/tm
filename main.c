#include <assert.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "helpers.h"

#define TASK_DIR "tasks"

typedef struct {
  char path[512];
  char title[128];
  int priority;
  char date[20];
} Task;

typedef struct {
  Task *items;
  size_t capacity;
  size_t count;
} Tasks;

void cmd_help() {
  printf("Usage: tm [-h | --help] <command>\n"
         "\n"
         "Commands:\n"
         "    init              create tasks folder under current directory\n"
         "    add title         add a task\n"
         "    list [--done]     list tasks sorted by priority, undone ones by default\n");
}

void cmd_init() {
  if (mkdir(TASK_DIR, 0755) == 0)
    printf("Initialized %s folder.\n", TASK_DIR);
  else
    perror("Error");
}

void cmd_add(const char *name) {
  char timestamp[20], folder_path[512], file_path[512];
  time_t now = time(NULL);
  strftime(timestamp, sizeof(timestamp), "%Y%m%d-%H%M%S", localtime(&now));

  snprintf(folder_path, sizeof(folder_path), "%s/%s", TASK_DIR, timestamp);
  if (mkdir(folder_path, 0755) != 0) {
    perror("Mkdir failed");
    return;
  }

  snprintf(file_path, sizeof(file_path), "%s/README.md", folder_path);
  FILE *f = fopen(file_path, "w");
  if (!f)
    return;

  fprintf(f, "---\ntitle: %s\npriority: 50\ndone: false\n---\n", name);
  fclose(f);
  printf("Task created: %s\n", name);
  printf("%s\n", timestamp);
  printf("%s:2: %s\n", file_path, name);
}

int compare_tasks(const void *a, const void *b) {
  return ((Task *)b)->priority - ((Task *)a)->priority;
}

void cmd_list(bool show_done) {
  struct dirent *entry;
  DIR *dp = opendir(TASK_DIR);
  if (!dp) {
    perror("No tasks folder");
    return;
  }

  Tasks tasks = {0};

  while ((entry = readdir(dp))) {
    if (entry->d_name[0] == '.')
      continue;

    char path[512];
    snprintf(path, sizeof(path), "%s/%s/README.md", TASK_DIR, entry->d_name);

    FILE *f = fopen(path, "r");
    if (f) {
      char line[256];
      char title[128] = "Untitled";
      int priority = 0;
      int task_done = 0;

      while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "title: ", 7) == 0) {
          line[strcspn(line, "\n")] = 0;
          strncpy(title, line + 7, sizeof(title) - 1);
        } else if (strncmp(line, "priority: ", 10) == 0) {
          priority = atoi(line + 10);
        } else if (strncmp(line, "done: ", 6) == 0) {
          task_done = (strstr(line, "true") != NULL);
        }
      }
      fclose(f);

      if (task_done == show_done) {
        Task task = {0};
        strcpy(task.path, path);
        strcpy(task.title, title);
        task.priority = priority;
        strncpy(task.date, entry->d_name, 20);

        da_append(&tasks, task);
      }
    }
  }
  closedir(dp);

  qsort(tasks.items, tasks.count, sizeof(Task), compare_tasks);

  da_foreach(Task, task, &tasks) {
    printf("%s:2: %s [%d]\n", task->path, task->title, task->priority);
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cmd_help();
    return 1;
  }

  if (strcmp(argv[1], "init") == 0)
    cmd_init();
  else if (strcmp(argv[1], "add") == 0 && argc > 2)
    cmd_add(argv[2]);
  else if (strcmp(argv[1], "list") == 0) {
    bool show_done = (argc > 2 && strcmp(argv[2], "--done") == 0);
    cmd_list(show_done);
  } else {
    cmd_help();
    return (strcmp(argv[1], "-h") != 0 && strcmp(argv[1], "--help") == 0);
  }

  return 0;
}
