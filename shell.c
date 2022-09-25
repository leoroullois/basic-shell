#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_CHAR 1000
#define MAX_ARGS 50

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

void handlePipe(const int i, char *args[]) {
  args[i] = NULL;

  int pipe1[2];

  if (pipe(pipe1) == -1) {
    perror("pipe");
    exit(1);
  }

  pid_t pid1 = fork();
  int status1;
  switch (pid1) {
  case -1:
    perror("fork pipe 1");
    exit(2);
  case 0:
    close(1);
    dup(pipe1[1]);
    close(pipe1[0]);

    execvp(args[0], args);
    exit(0);
    break;

  default: {
    pid_t pid2 = fork();
    int status2;
    switch (pid2) {
    case -1:
      perror("fork pipe 2");
      exit(3);
      break;
    case 0:

      close(0);
      dup(pipe1[0]);
      close(pipe1[1]);

      execvp(args[i + 1], &args[i + 1]);

      exit(0);
      break;
    default:
      close(pipe1[0]);
      close(pipe1[1]);
      waitpid(pid1, &status1, 0);
      waitpid(pid2, &status2, 0);

      exit(0);
      break;
    }
    exit(0);
    break;
  }
  }
}

void executeCommand(char *args[]) {
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
    } else if (strcmp(args[i], "|") == 0) {
      handlePipe(i, args);
    }
    i++;
  }

  if (execvp(args[0], args) < 0) {
    printf("Command not found\n");
  }

  if (strcmp(args[0], "cd")) {
    // TODO: handle cd
  }
}

int main(int argc, char *argv[]) {
  while (1) {
    pid_t pid = fork();
    int status;
    switch (pid) {
    case -1:
      perror("fork");
      exit(1);
    case 0:
      printf("> ");
      char command[MAX_CHAR];
      char *args[MAX_ARGS];
      fgets(command, MAX_ARGS, stdin);
      split(command, args);

      executeCommand(args);

      exit(0);
    default:
      waitpid(pid, &status, 0);
      continue;
    }
  }
  return 0;
}
