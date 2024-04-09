#ifndef INC_PROCESS_JOBS
#define INC_PROCESS_JOBS

int first_child (int read_fd, int write_fd);
int second_child (int read_fd);
int parent (int write_fd1, int write_fd2);


#endif /* INC_PROCESS_JOBS */
