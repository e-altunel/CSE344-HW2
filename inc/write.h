#ifndef INC_WRITE
#define INC_WRITE


void write_char (char *buffer, char c, int *index, int len);
void write_string (char *buffer, const char *str, int *index, int len);
void write_int (char *buffer, int n, int *index, int len);
void process_safe_write (int fd, const char *format, ...);

#endif /* INC_WRITE */
