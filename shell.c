#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int status;
  char command[50];
  while (1) {
    pid_t pid = fork();
    switch (pid) {
    case -1:
      perror("fork");
      exit(1);
    case 0:
      printf("> ");

      scanf("%s", command);
      execlp(command, command, NULL, NULL);
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
