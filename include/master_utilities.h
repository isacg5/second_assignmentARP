#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

int flag = 0;

void sig_handler(int signo)
{
    if (signo == SIGUSR1){
      flag = 1;
    }
}

int spawn(const char *program, char *arg_list[])
{
  // Creates a child and execute the executable passed as argument
  pid_t child_pid = fork();

  if (child_pid < 0)
  {
    perror("Error while forking...");
    return -1;
  }

  else if (child_pid != 0)
    return child_pid;

  else
  {
    if (execvp(program, arg_list) == 0)
    {
      perror("Exec failed");
      return -1;
    }
  }

  return 0;
}