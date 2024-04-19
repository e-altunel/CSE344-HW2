#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <write.h>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif

void write_char(char* buffer, char c, int* index, int len) {
  if (*index + 1 < len) buffer[(*index)++] = c;
}

void write_string(char* buffer, const char* str, int* index, int len) {
  if (*index + (int)strlen(str) < len) {
    for (size_t i = 0; i < strlen(str); i++) {
      buffer[(*index)++] = str[i];
    }
  }
}

/**
 * @brief Write an integer to a buffer recursively
 * 
 * @param buffer The buffer to write to
 * @param n The integer to write
 * @param index The index of the buffer
 * @param len The length of the buffer
 */
static void write_int_helper(char* buffer, int n, int* index, int len) {
  if (n == -2147483648) {
    write_string(buffer, "-2147483648", index, len);
    return;
  }
  if (n < 0) {
    write_char(buffer, '-', index, len);
    n = -n;
  }
  if (n < 10) {
    write_char(buffer, n + '0', index, len);
    return;
  }
  write_int_helper(buffer, n / 10, index, len);
  write_char(buffer, n % 10 + '0', index, len);
}

void write_int(char* buffer, int n, int* index, int len) {
  if (*index + 12 < len) write_int_helper(buffer, n, index, len);
}

void write_int_array(char* buffer, int* arr, int n, int* index, int len) {
  for (int i = 0; i < n; i++) {
    write_int(buffer, arr[i], index, len);
    if (i != n - 1) write_string(buffer, ", ", index, len);
  }
}

void write_style(char* buffer, style_t style, int* index, int len) {
  switch (style) {
    case ERROR:
      write_string(buffer, "\033[1;31m", index, len);
      break;
    case RESET:
      write_string(buffer, "\033[0m", index, len);
      break;
  }
}

void process_safe_write(int fd, const char* format, ...) {
  va_list args;
  va_start(args, format);
  int  index                   = 0;
  int  is_style                = 0;
  char buffer[BUFFER_SIZE + 1] = {0};

  int* array      = 0;
  int  array_size = 0;
  while (*format != '\0') {
    if (*format == '%') {
      format++;
      switch (*format) {
        case 's':
          write_string(buffer, va_arg(args, const char*), &index, BUFFER_SIZE);
          break;
        case 'c':
          write_char(buffer, va_arg(args, int), &index, BUFFER_SIZE);
          break;
        case 'd':
          write_int(buffer, va_arg(args, int), &index, BUFFER_SIZE);
          break;
        case 'a':
          array      = va_arg(args, int*);
          array_size = va_arg(args, int);
          write_int_array(buffer, array, array_size, &index, BUFFER_SIZE);
          break;
        case 'e':
          is_style = 1;
          write_style(buffer, ERROR, &index, BUFFER_SIZE);
          break;
        case 'r':
          is_style = 1;
          write_style(buffer, RESET, &index, BUFFER_SIZE);
          break;
        default:
          write_string(buffer, "%% BAD FORMAT %%", &index, BUFFER_SIZE);
          break;
      }
    } else {
      write_char(buffer, *format, &index, BUFFER_SIZE);
    }
    format++;
  }
  if (is_style) write_style(buffer, RESET, &index, BUFFER_SIZE);
  (void)!write(fd, buffer, index);
  fsync(fd);
  va_end(args);
}

int str2uint(const char* str) {
  int result = 0;
  while (*str != '\0') {
    if (*str < '0' || *str > '9') return -1;
    result = result * 10 + *str - '0';
    str++;
  }
  return result;
}

char* strdup_c(const char* s) {
  char* p = malloc(strlen(s) + 1);
  if (p != NULL) strcpy(p, s);
  return p;
}