#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int p1[2];
  int p2[2];
  if (pipe(p1) < 0 || pipe(p2) < 0) {
    fprintf(2, "pingpong: pipe failed\n");
    exit(1);
  }
  int pid = fork();
  if (pid < 0) {
    fprintf(2, "pingpong: fork failed\n");
    exit(1);
  }
  char buf[2] = {0};
  if (pid > 0) { // parent process
    close(p1[0]);
    close(p2[1]);
    write(p1[1], "a", 1);
    read(p2[0], buf, sizeof buf);
    fprintf(1, "%d: received pong\n", getpid());
    close(p1[1]);
    close(p2[0]);
  } else { // child process
    close(p1[1]);
    close(p2[0]);
    read(p1[0], buf, sizeof buf);
    fprintf(1, "%d: received ping\n", getpid());
    write(p2[1], "b", 1);
    close(p1[0]);
    close(p2[1]);
  }
  wait(0);
  exit(0);
}
