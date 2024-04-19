#define _POSIX_C_SOURCE 1

#include <fcntl.h>
#include <macros.h>
#include <process_jobs.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <write.h>

int          child_count = 2;         // Number of children
static pid_t pid[2]      = {-1, -1};  // PIDs of children

/**
 * @brief Kill all children
 * 
 * @return int  
 */
int kill_children() {
  for (int i = 0; i < 2; i++) {
    if (pid[i] > 0) {
      if (kill(pid[i], SIGTERM) == 0)
        process_safe_write(1,
                           "%s Sent termination signal to child with PID %d\n",
                           PARENT_NAME, pid[i]);
    }
  }
  return 0;
}

/**
 * @brief Signal handler for SIGCHLD
 * 
 * @param signal The signal number
 */
void sigchld_handler(int signal) {
  if (signal != SIGCHLD) {
    return;
  }
  pid_t pid_child;
  int   status;
  int   return_value;

  while ((pid_child = waitpid(-1, &status, WNOHANG)) > 0) {
    return_value = WEXITSTATUS(status);
    child_count--;
    if (return_value == 0) {
      process_safe_write(1, "%s Child with PID %d exited with status %d\n",
                         PARENT_NAME, pid_child, return_value);
    } else {
      if (return_value != SELF_EXIT)
        process_safe_write(1,
                           "%s Child with PID %d exited with status %d "
                           "which is not a success status\n",
                           PARENT_NAME, pid_child, return_value);
      if (child_count > 0) {
        process_safe_write(1, "%s Killing remaining children\n", PARENT_NAME);
        kill_children();
        int status_remaining;
        while (waitpid(-1, &status_remaining, 0) > 0)
          ;
        unlink_fifos();
        clear_all();
        process_safe_write(1, "%s Exiting due to error\n", PARENT_NAME);
        exit(0);
      }
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc != 1 && argc != 2) {
    process_safe_write(2,
                       "Usage: %s [0 < number of random numbers < 10]\nDefault "
                       "number of random numbers is 5\n",
                       argv[0]);
    return 1;
  }
  int numberOfRandomNumbers = argc == 1 ? 5 : str2uint(argv[1]);
  ASSERT(numberOfRandomNumbers > 0 && numberOfRandomNumbers < 10, PARENT_NAME,
         " Number of random numbers must be between 0 and 10\n", 1);

  ASSERT(open_fifos() == 0, PARENT_NAME, "Error opening fifos\n", 1);

  if ((pid[0] = fork()) == 0) {
    return first_child();
  } else if ((pid[1] = fork()) == 0) {
    return second_child();
  } else if (pid[0] == -1 || pid[1] == -1) {
    process_safe_write(2, "%s Error forking\n", PARENT_NAME);
    process_safe_write(1, "%s Killing children if any\n", PARENT_NAME);
    kill_children();
    ASSERT(unlink_fifos() == 0, PARENT_NAME, "Error unlinking fifos\n", 2);
  } else {
    if (parent(numberOfRandomNumbers) == -1) {
      process_safe_write(2, "%s Error in parent\n", PARENT_NAME);
      process_safe_write(1, "%s Killing children\n", PARENT_NAME);
      kill_children();
      ASSERT(unlink_fifos() == 0, PARENT_NAME, "Error unlinking fifos\n", 2);
    }
  }

  return 0;
}
