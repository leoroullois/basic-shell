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

void handlePsSequence(const int i, char *args[]) {
  args[i] = NULL;
  pid_t pid2 = fork();
  int status2;
  switch (pid2) {
  case -1:
    perror("fork");
    exit(1);
  case 0:
    execvp(args[0], args);
    exit(0);
    break;
  default:
    waitpid(pid2, &status2, 0);
    execvp(args[i + 1], &args[i + 1]);
    exit(0);
    break;
  }
}

void handlePsParallel(const int i, char *args[]) {
  args[i] = NULL;
  pid_t pid2 = fork();
  switch (pid2) {
  case -1:
    perror("fork");
    exit(1);
  case 0:
    execvp(args[i + 1], &args[i + 1]);
    exit(0);
  default:
    execvp(args[0], args);
    exit(0);
    break;
  }
}

void handlePsAnd(const int i, char *args[]) {
  args[i] = NULL;
  pid_t pidPs = fork();
  int statusPs;
  switch (pidPs) {
  case -1:
    perror("fork");
    exit(1);
  case 0:
    execvp(args[0], args);
    exit(0);
  default:
    waitpid(pidPs, &statusPs, 0);
    if (statusPs == 0) {
      execvp(args[i + 1], &args[i + 1]);
    }
    exit(0);
    break;
  }
}

void handlePsOr(const int i, char *args[]) {
  args[i] = NULL;
  pid_t pidPs = fork();
  int statusPs;
  switch (pidPs) {
  case -1:
    perror("fork");
    exit(1);
  case 0:
    execvp(args[0], args);
    exit(0);
  default:
    waitpid(pidPs, &statusPs, 0);
    if (statusPs > 0) {
      execvp(args[i + 1], &args[i + 1]);
    }
    exit(0);
    break;
  }
}

int main(int argc, char *argv[]) {
  char command[MAX];
  char *args[MAX];
  int status;

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
        } else if (strcmp(args[i], ";") == 0) {
          handlePsSequence(i, args);
        } else if (strcmp(args[i], "&") == 0) {
          handlePsParallel(i, args);
        } else if (strcmp(args[i], "&&") == 0) {
          handlePsAnd(i, args);
        } else if (strcmp(args[i], "||") == 0) {
          handlePsOr(i, args);
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
