#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// returns 0 if read failed
int
read_number(const int fd)
{
  int number = 0;
  const int read_bytes = read(fd, &number, sizeof number);
  if (read_bytes < 0) {
    fprintf(2, "primes: process %d read from fd %d failed\n", getpid(), fd);
    exit(1);
  }
  return read_bytes <= 0 ? 0 : number;
}

int process_main(const int fd1);

// return a file descriptor to send number to child process
int
create_process()
{
  int p[2];
  if (pipe(p) < 0) {
    fprintf(2, "primes: process %d pipe failed\n", getpid());
    exit(1);
  }
  const int pid = fork();
  if (pid < 0) {
    fprintf(2, "primes: process %d fork failed\n", getpid());
    exit(1);
  }
  if (pid == 0) { // child process
    close(p[1]);
    process_main(p[0]); // will exit
  } else { // parent process
    close(p[0]);
  }
  return p[1]; // only parent process steps here
}

int
process_main(const int fdr)
{
  const int prime_number = read_number(fdr);
  if (prime_number == 0) {
    exit(0);
  }
  fprintf(1, "prime %d\n", prime_number);
  const int fdw = create_process();
  for (int number = 0; (number = read_number(fdr)); ) {
    if (number % prime_number && write(fdw, &number, sizeof number) <= 0) {
      fprintf(2, "primes: process %d write to fd %d failed\n", getpid(), fdw);
      close(fdr);
      close(fdw);
      wait(0);
      exit(1);
    }
  }
  close(fdr);
  close(fdw);
  wait(0);
  exit(0);
}

int
main(int argc, char *argv[])
{
  int fdw = create_process();
  for (int number = 2; number <= 35; ++number) {
    if (write(fdw, &number, sizeof number) <= 0) {
      fprintf(2, "primes: process %d write to fd %d failed\n", getpid(), fdw);
      close(fdw);
      wait(0);
      exit(1);
    }
  }
  close(fdw);
  wait(0);
  exit(0);
}
