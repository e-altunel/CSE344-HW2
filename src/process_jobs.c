#define _POSIX_C_SOURCE 1

#include <errno.h>
#include <fcntl.h>
#include <macros.h>
#include <process_jobs.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <write.h>

int first_child() {
  int fd1 = open(fifo1, O_RDONLY);
  ASSERT_GOTO(fd1 != -1, "Error opening fifo1\n", Error_3);

  int fd2 = open(fifo2, O_WRONLY);
  ASSERT_GOTO(fd2 != -1, "Error opening fifo2\n", Error_2);

  sleep(10);

  int numberOfRandomNumbers;
  ASSERT_GOTO(read(fd1, &numberOfRandomNumbers, sizeof(int)) != -1,
              "Error reading number of random numbers\n", Error_1);

  int* randomNumbers = (int*)calloc(numberOfRandomNumbers, sizeof(int));
  ASSERT_GOTO(randomNumbers != NULL, "Error allocating memory\n", Error_1);
  ASSERT_GOTO(
      read(fd1, randomNumbers, numberOfRandomNumbers * sizeof(int)) != -1,
      "Error reading random numbers\n", Error_0);

  int sum = 0;
  for (int i = 0; i < numberOfRandomNumbers; i++)
    sum += randomNumbers[i];
  free(randomNumbers);

  ASSERT_GOTO(write(fd2, &sum, sizeof(int)) != -1, "Error writing sum\n",
              Error_0);

  close(fd1);
  close(fd2);

  process_safe_write(1, "[First Child] Sum of random numbers: %d\n", sum);
  process_safe_write(1, "[First Child] Exiting\n");
  return 0;
Error_0:
  free(randomNumbers);
Error_1:
  close(fd2);
Error_2:
  close(fd1);
Error_3:
  return -1;
}

int second_child() {
  int fd2 = open(fifo2, O_RDONLY);
  ASSERT_GOTO(fd2 != -1, "Error opening fifo2\n", Error_3);

  int commandLength;
  ASSERT_GOTO(read(fd2, &commandLength, sizeof(int)) != -1,
              "Error reading command length\n", Error_2);

  char* command = (char*)calloc(commandLength + 1, sizeof(char));
  ASSERT_GOTO(command != NULL, "Error allocating memory\n", Error_2);
  ASSERT_GOTO(read(fd2, command, commandLength) != -1,
              "Error reading command\n", Error_1);
  command[commandLength] = '\0';

  int numberOfRandomNumbers;
  ASSERT_GOTO(read(fd2, &numberOfRandomNumbers, sizeof(int)) != -1,
              "Error reading number of random numbers\n", Error_1);
  int* randomNumbers = (int*)calloc(numberOfRandomNumbers, sizeof(int));
  ASSERT_GOTO(randomNumbers != NULL, "Error allocating memory\n", Error_1);
  ASSERT_GOTO(
      read(fd2, randomNumbers, numberOfRandomNumbers * sizeof(int)) != -1,
      "Error reading random numbers\n", Error_0);

  int sum = 0;
  ASSERT_GOTO(read(fd2, &sum, sizeof(int)) != -1, "Error reading sum\n",
              Error_0);
  close(fd2);

  process_safe_write(1, "[Second Child] Received sum: %d\n", sum);
  int result = 0;
  if (strcmp(command, "multiply") == 0) {
    result = 1;
    for (int i = 0; i < numberOfRandomNumbers; i++) {
      result *= randomNumbers[i];
    }
    process_safe_write(1, "[Second Child] Product of random numbers: %d\n",
                       result);
    process_safe_write(1, "[Second Child] Sum of two children's results: %d\n",
                       result + sum);
  } else {
    process_safe_write(1, "[Second Child] Unknown command: %s\n", command);
  }
  free(command);
  free(randomNumbers);
  process_safe_write(1, "[Second Child] Exiting\n");
  return 0;
Error_0:
  free(randomNumbers);
Error_1:
  free(command);
Error_2:
  close(fd2);
Error_3:
  return -1;
}

static int child_count = 2;

static void sigchld_handler(int signal) {
  (void)signal;
  pid_t pid;
  int status;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    child_count--;
    if (WIFEXITED(status)) {
      process_safe_write(1, "Child with PID %d exited with status %d\n", pid,
                         WEXITSTATUS(status));
    }
  }
}

int parent(int numberOfRandomNumbers) {
  struct sigaction sa;
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGCHLD, &sa, NULL);

  int fd1 = open(fifo1, O_WRONLY);
  ASSERT_GOTO(fd1 != -1, "Error opening fifo1\n", Error_3);

  int fd2 = open(fifo2, O_WRONLY);
  ASSERT_GOTO(fd2 != -1, "Error opening fifo2\n", Error_2);

  int* randomNumbers = (int*)calloc(numberOfRandomNumbers, sizeof(int));
  ASSERT_GOTO(randomNumbers != NULL, "Error allocating memory\n", Error_1);

  srand(time(NULL));
  for (int i = 0; i < numberOfRandomNumbers; i++)
    randomNumbers[i] = rand() % 10 + 1;
  process_safe_write(1, "[Parent] Generated random numbers: %a\n",
                     randomNumbers, numberOfRandomNumbers);
  ASSERT_GOTO(write(fd1, &numberOfRandomNumbers, sizeof(int)) != -1,
              "Error writing number of random numbers\n", Error_0);
  ASSERT_GOTO(
      write(fd1, randomNumbers, numberOfRandomNumbers * sizeof(int)) != -1,
      "Error writing random numbers\n", Error_0);

  const char* command = "multiply";
  int commandLength = strlen(command);
  ASSERT_GOTO(write(fd2, &commandLength, sizeof(int)) != -1,
              "Error writing command length\n", Error_0);
  ASSERT_GOTO(write(fd2, command, commandLength) != -1,
              "Error writing command\n", Error_0);
  ASSERT_GOTO(write(fd2, &numberOfRandomNumbers, sizeof(int)) != -1,
              "Error writing number of random numbers\n", Error_0);
  ASSERT_GOTO(
      write(fd2, randomNumbers, numberOfRandomNumbers * sizeof(int)) != -1,
      "Error writing random numbers\n", Error_0);
  free(randomNumbers);
  close(fd1);

  int seconds = 0;
  while (child_count > 0) {
    process_safe_write(
        1,
        "[Parent] Waiting for children to finish, waited %d seconds, %d "
        "children remaining\n",
        seconds, child_count);
    sleep(2);
    seconds += 2;
  }

  close(fd2);
  return 0;
Error_0:
  free(randomNumbers);
Error_1:
  close(fd2);
Error_2:
  close(fd1);
Error_3:
  return -1;
}

int open_fifos() {
  if (mkfifo(fifo1, 0666) == -1) {
    if (errno != EEXIST) {
      process_safe_write(2, "Error creating fifo1\n");
      return -1;
    }
  }

  if (mkfifo(fifo2, 0666) == -1) {
    if (errno != EEXIST) {
      process_safe_write(2, "Error creating fifo2\n");
      if (unlink(fifo1) == -1) {
        process_safe_write(2, "Error unlinking fifo1\n");
      }
      return -1;
    }
  }
  return 0;
}

int unlink_fifos() {
  int status = 0;
  if (unlink(fifo1) == -1) {
    process_safe_write(2, "Error unlinking fifo1\n");
    status = -1;
  }

  if (unlink(fifo2) == -1) {
    process_safe_write(2, "Error unlinking fifo2\n");
    status = -1;
  }
  return status;
}