#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX 50

int split(char *buffer, char **word) {
  char *t = buffer;
  int i = 0;
  while (word[i++] = strtok(t, " \f\n\r\t\v"))
    t = NULL;
  return i;
}

int main(int argc, char *argv[]) {
  int status;
  char command[MAX];

  while (1) {
    pid_t pid = fork();
    switch (pid) {
    case -1:
      perror("fork");
      exit(1);
    case 0:
      printf("> ");
      fgets(command, MAX, stdin);

      char *args[MAX];
      split(command, args);
      execvp(command, args);
      exit(0);

    default:
      waitpid(pid, &status, 0);
      break;
    }
  }
  // execvp("ls", argv);
  printf("sortie\n");
  return 0;
}
