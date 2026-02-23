#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define TASK_DIR "tasks"

typedef struct {
  char path[512];
  char title[128];
  int priority;
  char date[20];
} Task;

void cmd_help() {
  printf("Usage: tm [-h | --help] <command>\n"
         "\n"
         "Commands:\n"
         "    init              create tasks folder under current directory\n"
         "    add title         add a task\n"
         "    list              list undone tasks sorted by priority\n");
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
  printf("Task created: %s\n", folder_path);
}

int compare_tasks(const void *a, const void *b) {
  return ((Task *)b)->priority - ((Task *)a)->priority;
}

void cmd_list() {
  struct dirent *entry;
  DIR *dp = opendir(TASK_DIR);
  if (!dp) {
    perror("No tasks folder");
    return;
  }

  Task tasks[256];
  int count = 0;

  while ((entry = readdir(dp)) && count < 256) {
    if (entry->d_name[0] == '.')
      continue;

    snprintf(tasks[count].path, 512, "%s/%s/README.md", TASK_DIR,
             entry->d_name);
    strncpy(tasks[count].date, entry->d_name, 20);

    FILE *f = fopen(tasks[count].path, "r");
    if (f) {
      char line[256];
      while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "title: ", 7) == 0) {
          line[strcspn(line, "\n")] = 0;
          strcpy(tasks[count].title, line + 7);
        } else if (strncmp(line, "priority: ", 10) == 0) {
          tasks[count].priority = atoi(line + 10);
        }
      }
      fclose(f);
      count++;
    }
  }
  closedir(dp);

  qsort(tasks, count, sizeof(Task), compare_tasks);

  for (int i = 0; i < count; i++) {
    printf("%s:2: (%3d) %s\n", tasks[i].path, tasks[i].priority,
           tasks[i].title);
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
  else if (strcmp(argv[1], "list") == 0)
    cmd_list();
  else {
    cmd_help();
    return (strcmp(argv[1], "-h") != 0 && strcmp(argv[1], "--help") == 0);
  }

  return 0;
}
