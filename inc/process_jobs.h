#ifndef INC_PROCESS_JOBS
#define INC_PROCESS_JOBS

#define fifo1 "fifo1"
#define fifo2 "fifo2"

int first_child();
int second_child();
int parent(int numberOfRandomNumbers);
int open_fifos();
int unlink_fifos();

#endif /* INC_PROCESS_JOBS */
