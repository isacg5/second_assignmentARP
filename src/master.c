#include "./../include/master_utilities.h"

int main(int argc, char const *argv[])
{
  if (signal(SIGUSR1, sig_handler) == SIG_ERR)
    printf("\ncan't catch SIGINT\n");

  char *arg_list_A[] = {"/usr/bin/konsole", "-e", "./bin/processA", NULL};
  char *arg_list_B[] = {"/usr/bin/konsole", "-e", "./bin/processB", NULL};

  pid_t pid_procA;
  if ((pid_procA = spawn("/usr/bin/konsole", arg_list_A)) == -1)
    return -1;

  // Wait until process A send a signal, because process A is always executed
  // before process B, since process A is the one who created the semaphores
  while (flag == 0)
  {
  }

  pid_t pid_procB;
  if ((pid_procB = spawn("/usr/bin/konsole", arg_list_B)) == -1)
    return -1;

  int status;
  waitpid(pid_procA, &status, 0);
  waitpid(pid_procB, &status, 0);

  printf("Main program exiting with status %d\n", status);
  return 0;
}
