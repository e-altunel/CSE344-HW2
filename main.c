#include <macros.h>
#include <process_jobs.h>
#include <signal.h>
#include <unistd.h>
#include <write.h>

int main(int argc, char* argv[]) {
  if (argc != 1 && argc != 2) {
    process_safe_write(2,
                       "Usage: %s [0 < number of random numbers < 10]\nDefault "
                       "number of random numbers is 5\n",
                       argv[0]);
    return 1;
  }
  int numberOfRandomNumbers = argc == 1 ? 5 : str2uint(argv[1]);
  ASSERT(numberOfRandomNumbers > 0 && numberOfRandomNumbers < 10,
         "Number of random numbers must be between 0 and 10\n", 1);

  ASSERT(open_fifos() == 0, "Error opening fifos\n", 1);
  int pid[2] = {0, 0};

  if ((pid[0] = fork()) == 0) {
    ASSERT(first_child() == 0, "Error in first child\n", 2);
  } else if ((pid[1] = fork()) == 0) {
    ASSERT(second_child() == 0, "Error in second child\n", 2);
  } else if (pid[0] == -1 || pid[1] == -1) {
    process_safe_write(2, "Error forking\n");
    ASSERT(unlink_fifos() == 0, "Error unlinking fifos\n", 2);
  } else {
    if (parent(numberOfRandomNumbers) == -1) {
      process_safe_write(2, "Error in parent\n");
      ASSERT(unlink_fifos() == 0, "Error unlinking fifos\n", 2);
    }
  }

  return 0;
}
