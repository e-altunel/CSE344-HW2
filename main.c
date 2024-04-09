

#include <process_jobs.h>
#include <unistd.h>
#include <write.h>

int main () {
  int first_pipe[2];
  int second_pipe[2];
  int pid[2];

  (void)!pipe (first_pipe);
  (void)!pipe (second_pipe);

  pid[0] = fork ();

  if (pid[0] == 0) {
    close (first_pipe[1]);
    close (second_pipe[0]);
    return first_child (first_pipe[0], second_pipe[1]);
  }

  pid[1] = fork ();

  if (pid[1] == 0) {
    close (first_pipe[0]);
    close (first_pipe[1]);
    close (second_pipe[1]);
    return second_child (second_pipe[0]);
  }

  close (first_pipe[0]);
  close (second_pipe[0]);

  return parent (first_pipe[1], second_pipe[1]);
}
