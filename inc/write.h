/**
 * @file write.h
 * @author Emirhan Altunel
 * @brief Header file for the write module. Contains function declarations for writing to the console or a file.
 * @date 2024-04-19
 */
#ifndef INC_WRITE
#define INC_WRITE

/**
 * @brief Enumeration for different styles of text.
 */
typedef enum style_e {
  ERROR,
  RESET,
} style_t;

/**
 * @brief Writes a single character to the buffer.
 * 
 * @param buffer Buffer to write to.
 * @param c Character to write.
 * @param index Index of the buffer.
 * @param len Length of the buffer.
 * 
 * If the buffer is full, the function does nothing.
 * 
 * @return void
 */
void write_char(char* buffer, char c, int* index, int len);

/**
 * @brief Writes a string to the buffer.
 * 
 * @param buffer Buffer to write to.
 * @param str String to write.
 * @param index Index of the buffer.
 * @param len Length of the buffer.
 * 
 * If the buffer is full, the function does nothing.
 * 
 * @return void
 */
void write_string(char* buffer, const char* str, int* index, int len);

/**
 * @brief Writes an integer to the buffer.
 * 
 * @param buffer Buffer to write to.
 * @param n Integer to write.
 * @param index Index of the buffer.
 * @param len Length of the buffer.
 * 
 * It does not check length of the integer. 
 * If the MIN_INT is cannot be represented in the buffer, the function does nothing. 
 * 
 * @return void
 */
void write_int(char* buffer, int n, int* index, int len);

/**
 * @brief Writes an array of integers to the buffer.
 * 
 * @param buffer Buffer to write to.
 * @param arr Array of integers to write.
 * @param n Number of integers in the array.
 * @param index Index of the buffer.
 * @param len Length of the buffer.
 * 
 * If the buffer is full, the function only writes the part that fits.
 * It prints the integers in the array with a comma and a space between them.
 * 
 * @return void
 */
void write_int_array(char* buffer, int* arr, int n, int* index, int len);

/**
 * @brief Writes a style to the buffer.
 * 
 * @param buffer Buffer to write to.
 * @param style Style to write.
 * @param index Index of the buffer.
 * @param len Length of the buffer.
 * 
 * If the buffer is full, the function does nothing.
 * 
 * @return void
 */
void write_style(char* buffer, style_t style, int* index, int len);

/**
 * @brief Writes a formatted string to the file descriptor.
 * 
 * @param fd File descriptor to write to.
 * @param format Format string.
 * @param ... Arguments to the format string.
 * 
 * The function writes the formatted string to the file descriptor.
 * It uses a buffer to write to the file descriptor. 
 * It is process-safe.
 * 
 * @return void
 */
void process_safe_write(int fd, const char* format, ...);

/**
 * @brief Converts a string to an unsigned integer.
 * 
 * @param str String to convert.
 * 
 * The function converts the string to an unsigned integer.
 * 
 * @return The unsigned integer.
 */
int str2uint(const char* str);

/**
 * @brief Duplicates a string.
 * 
 * @param s String to duplicate.
 * 
 * The function duplicates the string.
 * 
 * @return The duplicated string. Use free() to free the memory.
 */
char* strdup_c(const char* s);

#endif /* INC_WRITE */
