/**
 * @file process_jobs.h
 * @brief Header file for the process jobs module.
 *
 * This file contains the declarations for the process jobs module, which is responsible for
 * handling and processing jobs of the parent and child processes.
 */
#ifndef INC_PROCESS_JOBS
#define INC_PROCESS_JOBS

#define fifo1 "fifo1" /** Name of the first FIFO */
#define fifo2 "fifo2" /** Name of the second FIFO */

#define PARENT_NAME \
  "\033[1;34m[Parent]\033[0m" /** Name of the parent process */
#define FIRST_CHILD_NAME \
  "\033[1;32m[First Child]\033[0m" /** Name of the first child process */
#define SECOND_CHILD_NAME \
  "\033[1;33m[Second Child]\033[0m" /** Name of the second child process */

#define SELF_EXIT 200 /** Exit status for self exit */

extern int child_count; /** Number of child processes */

int first_child();
int second_child();
int parent(int numberOfRandomNumbers);
int open_fifos();
int clear_all();
int unlink_fifos();
void sigchld_handler(int signal);
int kill_children();

#endif /* INC_PROCESS_JOBS */
