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

static int   fd1           = -1;   /** File descriptor for fifo1 */
static int   fd2           = -1;   /** File descriptor for fifo2 */
static int*  randomNumbers = NULL; /** Array of random numbers */
static char* command = NULL;   /** Command to be passed to the second child */
static int   child_number = 0; /** Number of the child process */

/**
 * @brief Get the signal name object
 * 
 * @param signal The signal number
 * @return const char* The name of the signal
 */
static const char* get_signal_name(int signal) {
  switch (signal) {
    case SIGTERM:
      return "SIGTERM";
    case SIGINT:
      return "SIGINT";
    case SIGPIPE:
      return "SIGPIPE";
    case SIGCHLD:
      return "SIGCHLD";
    default:
      return "Unknown signal";
  }
}

/**
 * @brief Signal handler for SIGCHLD
 * 
 * This function is called when a child process terminates or is killed.
 * 
 * @return int  0 on success, -1 on error
 */
int clear_all() {
  if (randomNumbers != NULL) {
    free(randomNumbers);
    randomNumbers = NULL;
  }
  if (command != NULL) {
    free(command);
    command = NULL;
  }
  if (fd1 != -1) {
    close(fd1);
    fd1 = -1;
  }
  if (fd2 != -1) {
    close(fd2);
    fd2 = -1;
  }
  return 0;
}

/**
 * @brief Signal handler for SIGTERM, SIGINT, and SIGPIPE ...
 * 
 * This function is called when the parent process or one of the children receives a termination signal.
 * It will kill all children and exit the parent process.
 * 
 * @param signal The signal number
 */
static void term_handler(int signal) {
  clear_all();
  process_safe_write(1, "%s with PID %d received termination signal %s\n",
                     child_number == 0   ? PARENT_NAME
                     : child_number == 1 ? FIRST_CHILD_NAME
                                         : SECOND_CHILD_NAME,
                     getpid(), get_signal_name(signal));
  if (child_number != 0) exit(SELF_EXIT);
  process_safe_write(1, "%s Killing remaining children\n", PARENT_NAME);
  kill_children();
  int status_remaining;
  while (waitpid(-1, &status_remaining, 0) > 0)
    ;
  unlink_fifos();
  process_safe_write(1, "%s Exiting due to error\n", PARENT_NAME);
  exit(0);
}

/**
 * @brief The job of the first child process
 * 
 * This function is called when the first child process is created.
 * It will read the random numbers from the fifo1, calculate the sum, and write it to fifo2.
 * 
 * @return int 0 on success, -1 on error
 */
int first_child() {
  child_number = 1;

  /**
   * @brief Signal handler for SIGTERM, SIGINT, and SIGPIPE ...
   * 
   */
  struct sigaction sa = {0};
  sa.sa_handler       = term_handler;
  ASSERT_GOTO(sigemptyset(&sa.sa_mask) != -1, FIRST_CHILD_NAME,
              "Error initializing signal mask\n", Error_3);
  ASSERT_GOTO(sigaction(SIGTERM, &sa, NULL) != -1, FIRST_CHILD_NAME,
              "Error setting signal handler\n", Error_3);
  ASSERT_GOTO(sigaction(SIGINT, &sa, NULL) != -1, FIRST_CHILD_NAME,
              "Error setting signal handler\n", Error_3);
  ASSERT_GOTO(sigaction(SIGPIPE, &sa, NULL) != -1, FIRST_CHILD_NAME,
              "Error setting signal handler\n", Error_3);

  /**
   * @brief Open the fifo1 and fifo2
   * 
   */
  fd1 = open(fifo1, O_RDONLY);
  ASSERT_GOTO(fd1 != -1, FIRST_CHILD_NAME, "Error opening fifo1\n", Error_3);

  fd2 = open(fifo2, O_WRONLY);
  ASSERT_GOTO(fd2 != -1, FIRST_CHILD_NAME, "Error opening fifo2\n", Error_2);

  /**
   * @brief Sleep for 10 seconds
   * 
   */
  sleep(10);

  /**
   * @brief Read the number of random numbers
   * 
   * @param numberOfRandomNumbers The number of random numbers
   * @param randomNumbers The array of random numbers
   */
  int numberOfRandomNumbers;
  ASSERT_GOTO(read(fd1, &numberOfRandomNumbers, sizeof(int)) != -1,
              FIRST_CHILD_NAME, "Error reading number of random numbers\n",
              Error_1);

  randomNumbers = (int*)calloc(numberOfRandomNumbers, sizeof(int));
  ASSERT_GOTO(randomNumbers != NULL, FIRST_CHILD_NAME,
              "Error allocating memory\n", Error_1);
  ASSERT_GOTO(
      read(fd1, randomNumbers, numberOfRandomNumbers * sizeof(int)) != -1,
      FIRST_CHILD_NAME, "Error reading random numbers\n", Error_0);

  /**
   * @brief Calculate the sum of the random numbers
   * 
   */
  int sum = 0;
  for (int i = 0; i < numberOfRandomNumbers; i++) sum += randomNumbers[i];
  free(randomNumbers);
  randomNumbers = NULL;
  ASSERT_GOTO(write(fd2, &sum, sizeof(int)) != -1, FIRST_CHILD_NAME,
              "Error writing sum\n", Error_0);

  /**
   * @brief Close the file descriptors
   * 
   */
  close(fd1);
  fd1 = -1;
  close(fd2);
  fd2 = -1;

  process_safe_write(1, "%s Sum of random numbers: %d\n", FIRST_CHILD_NAME,
                     sum);
  process_safe_write(1, "%s Exiting\n", FIRST_CHILD_NAME);
  return 0;

  /**
   * @brief Error handling
   * 
   */
Error_0:
  free(randomNumbers);
  randomNumbers = NULL;
Error_1:
  close(fd2);
  fd2 = -1;
Error_2:
  close(fd1);
  fd1 = -1;
Error_3:
  return -1;
}

/**
 * @brief The job of the second child process
 * 
 * This function is called when the second child process is created.
 * It will read the command, the random numbers, and the sum from the fifo2, calculate the result of the command, and write the sum of the two children * outputs to the stdout.
 * 
 * @return int 0 on success, -1 on error
 */
int second_child() {
  child_number = 2;

  /**
   * @brief Signal handler for SIGTERM, SIGINT, and SIGPIPE ...
   * 
   */
  struct sigaction sa = {0};
  sa.sa_handler       = term_handler;
  ASSERT_GOTO(sigemptyset(&sa.sa_mask) != -1, SECOND_CHILD_NAME,
              "Error initializing signal mask\n", Error_3);
  ASSERT_GOTO(sigaction(SIGTERM, &sa, NULL) != -1, SECOND_CHILD_NAME,
              "Error setting signal handler\n", Error_3);
  ASSERT_GOTO(sigaction(SIGINT, &sa, NULL) != -1, SECOND_CHILD_NAME,
              "Error setting signal handler\n", Error_3);
  ASSERT_GOTO(sigaction(SIGPIPE, &sa, NULL) != -1, SECOND_CHILD_NAME,
              "Error setting signal handler\n", Error_3);

  /**
   * @brief Open the fifo2
   * 
   */
  fd2 = open(fifo2, O_RDONLY);
  ASSERT_GOTO(fd2 != -1, SECOND_CHILD_NAME, "Error opening fifo2\n", Error_3);

  /**
   * @brief Read the command and the random numbers
   * 
   * @param commandLength The length of the command
   * @param command The command to be executed
   * @param numberOfRandomNumbers The number of random numbers
   * @param randomNumbers The array of random numbers
   */
  int commandLength = 0;
  ASSERT_GOTO(read(fd2, &commandLength, sizeof(int)) != -1, SECOND_CHILD_NAME,
              "Error reading command length\n", Error_2);
  ASSERT_GOTO(commandLength > 0, SECOND_CHILD_NAME, "Invalid command length\n",
              Error_2);

  command = (char*)calloc(commandLength + 1, sizeof(char));
  ASSERT_GOTO(command != NULL, SECOND_CHILD_NAME, "Error allocating memory\n",
              Error_2);
  ASSERT_GOTO(read(fd2, command, commandLength) != -1, SECOND_CHILD_NAME,
              "Error reading command\n", Error_1);
  command[commandLength] = '\0';

  int numberOfRandomNumbers;
  ASSERT_GOTO(read(fd2, &numberOfRandomNumbers, sizeof(int)) != -1,
              SECOND_CHILD_NAME, "Error reading number of random numbers\n",
              Error_1);
  randomNumbers = (int*)calloc(numberOfRandomNumbers, sizeof(int));
  ASSERT_GOTO(randomNumbers != NULL, SECOND_CHILD_NAME,
              "Error allocating memory\n", Error_1);
  ASSERT_GOTO(
      read(fd2, randomNumbers, numberOfRandomNumbers * sizeof(int)) != -1,
      SECOND_CHILD_NAME, "Error reading random numbers\n", Error_0);

  /**
   * @brief Try to read the sum from the fifo2
   * If the fifo is empty read will return 0, so we need to try again. Until we get a value different from 0. Which is the sum of the first child or 
   * -1 if there was an error.
   * 
   */
  int sum          = 0;
  int return_value = -1;
  do {
    return_value = read(fd2, &sum, sizeof(int));
    ASSERT_GOTO(return_value != -1, SECOND_CHILD_NAME, "Error reading sum\n",
                Error_0);
  } while (return_value == 0);
  close(fd2);
  fd2 = -1;
  process_safe_write(1, "%s Received sum: %d\n", SECOND_CHILD_NAME, sum);

  /**
   * @brief Calculate the result of the command
   * 
   */
  int result = 0;
  if (strcmp(command, "multiply") == 0) {
    result = 1;
    for (int i = 0; i < numberOfRandomNumbers; i++) {
      result *= randomNumbers[i];
    }
    process_safe_write(1, "%s Result of multiplication: %d\n",
                       SECOND_CHILD_NAME, result);
    process_safe_write(1, "%s Sum of two children's results: %d\n",
                       SECOND_CHILD_NAME, result + sum);
  } else {
    process_safe_write(2, "%s Invalid command: %s\n", SECOND_CHILD_NAME,
                       command);
    goto Error_0;
  }

  /**
   * @brief Free the memory and exit
   * 
   */
  free(command);
  command = NULL;
  free(randomNumbers);
  randomNumbers = NULL;

  process_safe_write(1, "%s Exiting\n", SECOND_CHILD_NAME);
  return 0;

  /**
   * @brief Error handling
   * 
   */
Error_0:
  free(randomNumbers);
  randomNumbers = NULL;
Error_1:
  free(command);
  command = NULL;
Error_2:
  close(fd2);
  fd2 = -1;
Error_3:
  return -1;
}

/**
 * @brief The job of the parent process
 * 
 * This function is called when the parent process is created.
 * It will generate random numbers, write them to fifo1, and send the command and the random numbers to the second child.
 * 
 * @param numberOfRandomNumbers The number of random numbers to generate
 * @return int 0 on success, -1 on error
 */
int parent(int numberOfRandomNumbers) {
  child_number = 0;

  /**
   * @brief Signal handler for SIGCHLD
   * 
   */
  struct sigaction sa = {0};
  sa.sa_handler       = sigchld_handler;
  ASSERT_GOTO(sigemptyset(&sa.sa_mask) != -1, PARENT_NAME,
              "Error initializing signal mask\n", Error_3);
  ASSERT_GOTO(sigaction(SIGCHLD, &sa, NULL) != -1, PARENT_NAME,
              "Error setting signal handler\n", Error_3);

  /**
   * @brief Signal handler for SIGTERM, SIGINT, and SIGPIPE ...
   * 
   */
  struct sigaction sa2 = {0};
  sa2.sa_handler       = term_handler;
  ASSERT_GOTO(sigemptyset(&sa2.sa_mask) != -1, PARENT_NAME,
              "Error initializing signal mask\n", Error_3);
  ASSERT_GOTO(sigaction(SIGTERM, &sa2, NULL) != -1, PARENT_NAME,
              "Error setting signal handler\n", Error_3);
  ASSERT_GOTO(sigaction(SIGINT, &sa2, NULL) != -1, PARENT_NAME,
              "Error setting signal handler\n", Error_3);
  ASSERT_GOTO(sigaction(SIGPIPE, &sa2, NULL) != -1, PARENT_NAME,
              "Error setting signal handler\n", Error_3);

  /**
   * @brief Open the fifo1 and fifo2
   * 
   */
  fd1 = open(fifo1, O_WRONLY);
  ASSERT_GOTO(fd1 != -1, PARENT_NAME, "Error opening fifo1\n", Error_3);
  fd2 = open(fifo2, O_WRONLY);
  ASSERT_GOTO(fd2 != -1, PARENT_NAME, "Error opening fifo2\n", Error_2);

  /**
   * @brief Generate random numbers
   * 
   */
  randomNumbers = (int*)calloc(numberOfRandomNumbers, sizeof(int));
  ASSERT_GOTO(randomNumbers != NULL, PARENT_NAME, "Error allocating memory\n",
              Error_1);
  srand(time(NULL));
  for (int i = 0; i < numberOfRandomNumbers; i++)
    randomNumbers[i] = rand() % 10 + 1;
  process_safe_write(1, "%s Generated random numbers: %a\n", PARENT_NAME,
                     randomNumbers, numberOfRandomNumbers);
  ASSERT_GOTO(write(fd1, &numberOfRandomNumbers, sizeof(int)) != -1,
              PARENT_NAME, "Error writing number of random numbers\n", Error_0);
  ASSERT_GOTO(
      write(fd1, randomNumbers, numberOfRandomNumbers * sizeof(int)) != -1,
      PARENT_NAME, "Error writing random numbers\n", Error_0);

  /**
   * @brief Send the command to the second child
   * 
   */
  command = strdup_c("multiply");
  ASSERT_GOTO(command != NULL, PARENT_NAME, "Error allocating memory\n",
              Error_0);
  int commandLength = strlen(command);
  ASSERT_GOTO(write(fd2, &commandLength, sizeof(int)) != -1, PARENT_NAME,
              "Error writing command length\n", Error_00);
  ASSERT_GOTO(write(fd2, command, commandLength) != -1, PARENT_NAME,
              "Error writing command\n", Error_00);
  ASSERT_GOTO(write(fd2, &numberOfRandomNumbers, sizeof(int)) != -1,
              PARENT_NAME, "Error writing number of random numbers\n",
              Error_00);
  ASSERT_GOTO(
      write(fd2, randomNumbers, numberOfRandomNumbers * sizeof(int)) != -1,
      PARENT_NAME, "Error writing random numbers\n", Error_00);

  /**
   * @brief Close the file descriptors and free the memory
   * 
   */
  free(command);
  command = NULL;
  free(randomNumbers);
  randomNumbers = NULL;
  close(fd1);
  fd1 = -1;
  close(fd2);
  fd2 = -1;

  /**
   * @brief Wait for the children to finish
   * 
   */
  int seconds = 0;
  while (child_count > 0) {
    process_safe_write(1,
                       "%s Waiting for children to finish, "
                       "waited %d seconds, %d "
                       "children remaining\n",
                       PARENT_NAME, seconds, child_count);
    sleep(2);
    seconds += 2;
  }
  process_safe_write(1, "%s Exiting\n", PARENT_NAME);

  return 0;

  /**
   * @brief Error handling
   * 
   */
Error_00:
  free(command);
  command = NULL;
Error_0:
  free(randomNumbers);
  randomNumbers = NULL;
Error_1:
  close(fd2);
  fd2 = -1;
Error_2:
  close(fd1);
  fd1 = -1;
Error_3:
  return -1;
}

/**
 * @brief Open the fifos
 * 
 * This function will open the fifos fifo1 and fifo2.
 * 
 * @return int 0 on success, -1 on error
 */
int open_fifos() {
  if (mkfifo(fifo1, 0666) == -1) {
    if (errno != EEXIST) {
      process_safe_write(2, "%s Error creating fifo1\n", PARENT_NAME);
      return -1;
    }
  }

  if (mkfifo(fifo2, 0666) == -1) {
    if (errno != EEXIST) {
      process_safe_write(2, "%s Error creating fifo2\n", PARENT_NAME);
      if (unlink(fifo1) == -1) {
        process_safe_write(2, "%s Error unlinking fifo1\n", PARENT_NAME);
      }
      return -1;
    }
  }
  return 0;
}

/**
 * @brief Unlink the fifos
 * 
 * This function will unlink the fifos fifo1 and fifo2.
 * 
 * @return int 0 on success, -1 on error
 */
int unlink_fifos() {
  int status = 0;
  if (unlink(fifo1) == -1) {
    process_safe_write(2, "%s Error unlinking fifo1\n", PARENT_NAME);
    status = -1;
  }

  if (unlink(fifo2) == -1) {

    status = -1;
  }
  return status;
}
