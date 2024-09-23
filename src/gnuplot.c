#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <concord/log.h>

#include "include/gnuplot.h"
#include "include/vector.h"

struct vector_char gnuplot_plot(char *expr, int *exit_status) {
  (void)expr;

  int pipefd[2];

  struct vector_char pngbuf;
  vector_init_char(&pngbuf);

  if (pipe(pipefd) == -1) {
    log_error("Pipe creation failed");
    *exit_status = EXIT_FAILURE;
    return pngbuf;
  }

  pid_t pid = fork();

  if (pid < 0) {
    log_error("Gnuplot fork failed");
    *exit_status = EXIT_FAILURE;
    return pngbuf;
  } else if (pid == 0) {
    close(pipefd[0]);

    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);

    char *eval_str = NULL;
    assert(asprintf(&eval_str, "set terminal png; plot %s", expr) != -1);
    execvp("gnuplot", (char *const[]){"gnuplot", "-e", eval_str, NULL});
    free(eval_str);
    exit(EXIT_FAILURE);
  }

  close(pipefd[1]);

  char buf;
  // HACK: this is slow
  while (read(pipefd[0], &buf, sizeof(buf)) > 0) {
    vector_push_char(&pngbuf, buf);
  }

  close(pipefd[0]);

  int status;
  waitpid(pid, &status, 0);

  if (WIFEXITED(status)) {
    int exit_code = WEXITSTATUS(status);
    *exit_status = exit_code;
    return pngbuf;
  }

  *exit_status = EXIT_FAILURE;
  return pngbuf;
}
