#include <stdarg.h>
#include <unistd.h>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif

void write_char (char *buffer, char c, int *index, int len) {
  if (*index + 1 < len)
    buffer[(*index)++] = c;
}

void write_string (char *buffer, const char *str, int *index, int len) {
  while (*str != '\0')
    write_char (buffer, *(str++), index, len);
}

static void write_int_helper (char *buffer, int n, int *index, int len) {
  if (n == -2147483648) {
    write_string (buffer, "-2147483648", index, len);
    return;
  }
  if (n < 0) {
    write_char (buffer, '-', index, len);
    n = -n;
  }
  if (n < 10) {
    write_char (buffer, n + '0', index, len);
    return;
  }
  write_int_helper (buffer, n / 10, index, len);
  write_char (buffer, n % 10 + '0', index, len);
}

void write_int (char *buffer, int n, int *index, int len) {
  if (*index + 12 < len)
    write_int_helper (buffer, n, index, len);
}

void process_safe_write (int fd, const char *format, ...) {
  va_list args;
  va_start (args, format);
  int index = 0;
  char buffer[BUFFER_SIZE];
  while (*format != '\0') {
    if (*format == '%') {
      format++;
      switch (*format) {
      case 's':
        write_string (buffer, va_arg (args, const char *), &index, BUFFER_SIZE);
        break;
      case 'c':
        write_char (buffer, va_arg (args, int), &index, BUFFER_SIZE);
        break;
      case 'd':
        write_int (buffer, va_arg (args, int), &index, BUFFER_SIZE);
        break;
      }
    } else {
      write_char (buffer, *format, &index, BUFFER_SIZE);
    }
    format++;
  }
  (void)!write (fd, buffer, index);
  fsync (fd);
  va_end (args);
}