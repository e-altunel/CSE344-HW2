#define _POSIX_C_SOURCE 1

#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <write.h>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif

int first_child (int read_fd, int write_fd) {
  sleep (5);
  char buffer[BUFFER_SIZE];
  int read_bytes     = read (read_fd, buffer, BUFFER_SIZE - 1);
  buffer[read_bytes] = '\0';
  process_safe_write (1, "First child received message: %s\n", buffer);
  process_safe_write (write_fd, "Hello from first child.");
  close (write_fd);
  return 0;
}

int second_child (int read_fd) {
  sleep (10);
  char buffer[BUFFER_SIZE];
  int read_bytes     = read (read_fd, buffer, BUFFER_SIZE - 1);
  buffer[read_bytes] = '\0';
  process_safe_write (1, "Second child received message: %s\n", buffer);
  return 0;
}

static int child_count = 2;

void sigchld_handler (int signal) {
  (void)signal;
  pid_t pid;
  int status;

  while ((pid = waitpid (-1, &status, WNOHANG)) > 0) {
    child_count--;
    if (WIFEXITED (status)) {
      process_safe_write (1, "Child with PID %d exited with status %d\n", pid,
                          WEXITSTATUS (status));
    } else if (WIFSIGNALED (status)) {
      process_safe_write (1, "Child with PID %d killed by signal %d\n", pid,
                          WTERMSIG (status));
    }
  }
}

int parent (int write_fd1, int write_fd2) {

  // Set up signal handler for SIGCHLD
  struct sigaction sa;
  sa.sa_handler = sigchld_handler;
  sigemptyset (&sa.sa_mask);
  sigaction (SIGCHLD, &sa, NULL);

  // Write to children
  int number_count = 10;
  srand (time (NULL));
  while (number_count > 0) {
    int temp = rand () % 100;
    process_safe_write (1, "Parent sending %d\n", temp);
    process_safe_write (write_fd1, "%d\n", temp);
    process_safe_write (write_fd2, "%d\n", temp);
    number_count--;
  }
  close (write_fd1);
  close (write_fd2);
  int seconds = 0;
  while (child_count > 0) {
    process_safe_write (1, "Waiting for children to finish, waited %d seconds, %d children remaining\n",
                        seconds, child_count);
    sleep (2);
    seconds += 2;
  }
  process_safe_write (1, "Parent exiting\n");
  return 0;
}