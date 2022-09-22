#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX 1000

int split(char *buffer, char **word) {
  char *t = buffer;
  int i = 0;
  while ((word[i++] = strtok(t, " \f\n\r\t\v")))
    t = NULL;
  return i;
}

// STDOUT redirections
void handleStdoutRedirection(const int i, char *args[]) {
  args[i] = NULL;
  close(1);
  open(args[i + 1], O_WRONLY | O_TRUNC | O_CREAT);
}

void handleStdoutAppendRedirection(const int i, char *args[]) {
  args[i] = NULL;
  close(1);
  open(args[i + 1], O_WRONLY | O_APPEND | O_CREAT);
}

// STDERR redirections
void handleStderrRedirection(const int i, char *args[]) {
  args[i] = NULL;
  close(2);
  open(args[i + 1], O_WRONLY | O_TRUNC | O_CREAT);
}

void handleStderrAppendRedirection(const int i, char *args[]) {
  args[i] = NULL;
  close(2);
  open(args[i + 1], O_WRONLY | O_APPEND | O_CREAT);
}

// STDIN redirections
void handleStdinRedirection(const int i, char *args[]) {
  args[i] = NULL;
  close(0);
  open(args[i + 1], O_RDONLY);
}

int main(int argc, char *argv[]) {
  int status;
  char command[MAX];
  char *args[MAX];

  while (1) {
    pid_t pid = fork();
    switch (pid) {
    case -1:
      perror("fork");
      exit(1);
    case 0:
      // Prompt
      printf("> ");
      fgets(command, MAX, stdin);
      split(command, args);

      int i = 0;
      while (args[i] != NULL) {
        if (strcmp(args[i], ">") == 0) {
          handleStdoutRedirection(i, args);
        } else if (strcmp(args[i], ">>") == 0) {
          printf("found");
          handleStdoutAppendRedirection(i, args);
        } else if (strcmp(args[i], "<") == 0) {
          handleStdinRedirection(i, args);
        } else if (strcmp(args[i], "2>") == 0) {
          handleStderrRedirection(i, args);
        } else if (strcmp(args[i], "2>>") == 0) {
          handleStderrAppendRedirection(i, args);
        } 
        i++;
      }
      if (execvp(args[0], args) < 0) {
        printf("Command not found\n");
      }
      exit(0);
    default:
      waitpid(pid, &status, 0);
      continue;
    }
  }
  return 0;
}
