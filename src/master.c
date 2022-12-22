#include "./../include/master_utilities.h"

int main(int argc, char const *argv[])
{
  char *arg_list_A[] = {"/usr/bin/konsole", "-e", "./bin/processA", NULL};
  char *arg_list_B[] = {"/usr/bin/konsole", "-e", "./bin/processB", NULL};

  pid_t pid_procA;
  if ((pid_procA = spawn("/usr/bin/konsole", arg_list_A)) == -1)
    return -1;

  // Sleep necessary to be sure that process A is always executed
  // before process B, since process A is the one who created the semaphores
  usleep(100000);
  
  pid_t pid_procB;
  if ((pid_procB = spawn("/usr/bin/konsole", arg_list_B)) == -1)
    return -1;

  int status;
  waitpid(pid_procA, &status, 0);
  waitpid(pid_procB, &status, 0);

  printf("Main program exiting with status %d\n", status);
  return 0;
}
