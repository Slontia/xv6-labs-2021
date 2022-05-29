#include <stdint.h>

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

void
append_arg(
    char* const arg_begin_p,
    char* const arg_end_p,
    const int xargs_fixed_argc,
    int* const xargs_addi_argc,
    const int xargs_max_argc,
    char* xargs_argv[],
    const int force_execute)
{
  if (arg_begin_p != arg_end_p) {
    if (xargs_fixed_argc + *xargs_addi_argc == MAXARG) {
      fprintf(2, "xargs: achieve max argument number %d\n", MAXARG);
      exit(1);
    }
    xargs_argv[xargs_fixed_argc + ((*xargs_addi_argc)++)] = arg_begin_p;
    *arg_end_p = 0; // truncate the word
  }
  if (*xargs_addi_argc == xargs_max_argc || (*xargs_addi_argc > 0 && force_execute)) {
    xargs_argv[xargs_fixed_argc + *xargs_addi_argc + 1] = 0;
    const int pid = fork();
    if (pid < 0) {
      fprintf(2, "xargs: fork failed\n");
      exit(1);
    }
    if (pid == 0) {
      exec(xargs_argv[0], xargs_argv);
      fprintf(2, "xargs: execute failed\n");
      exit(1);
    }
    wait(0);
    *xargs_addi_argc = 0;
  }
}

int
main(int argc, char *argv[])
{
  // parse argument n
  int n = INT32_MAX;
  int xargs_fixed_argc = argc - 1; // exclude "xargs"
  if (argc >= 2 && strcmp("-n", argv[1]) == 0) {
    if (argc == 2) {
      fprintf(2, "xargs: option requires an argument -- n\n");
      exit(1);
    }
    n = atoi(argv[2]);
    if (n <= 0) {
      fprintf(2, "xargs: argument should be positive -- n\n");
      exit(1);
    }
    xargs_fixed_argc -= 2; // exclude "-n xxx"
  }

  // fill fixed arguments
  char* xargs_argv[MAXARG + 1];
  for (int i = 0; i < xargs_fixed_argc; ++i) {
    xargs_argv[i] = argv[argc - xargs_fixed_argc + i];
  }

  // fill xargs arguments
  char buf[1025];
  int data_bytes = 0;
  int xargs_addi_argc = 0;
  for (;;) {
    const int read_bytes = read(0, buf + data_bytes, sizeof buf - 1 - data_bytes); // tail byte for set '\0'
    if (read_bytes < 0) {
      fprintf(2, "xargs: read failed\n");
      exit(1);
    }
    if (read_bytes == 0) { // EOF
      break;
    }
    data_bytes += read_bytes;
    if (sizeof buf <= data_bytes) {
      fprintf(2, "xargs: out of range %d\n", sizeof buf);
      exit(1);
    }
  }
  char* arg_p = buf;
  for (char* p = buf; p != buf + data_bytes; ++p) {
    if (*p != ' ' && *p != '\n' && *p != '\t' && *p != '\r') {
      continue;
    }
    append_arg(arg_p, p, xargs_fixed_argc, &xargs_addi_argc, n, xargs_argv, 0);
    arg_p = p + 1;
  }

  append_arg(arg_p, buf + data_bytes, xargs_fixed_argc, &xargs_addi_argc, n, xargs_argv, 1);

  exit(0);
}
