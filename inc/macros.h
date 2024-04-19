/**
 * @file macros.h
 * @author Emirhan Altunel
 * @brief This file contains a macro definition for ASSERT.
 * @date 2024-04-19
 */
#ifndef INC_MACROS
#define INC_MACROS

#include <stdlib.h>
#include <write.h>

/**
 * @file macros.h
 * @brief This file contains a macro definition for ASSERT.
 *
 * The ASSERT macro is used to check a condition and perform error handling if the condition is false.
 * It prints an error message along with the sender and message, and exits the program with the specified exit status.
 *
 * @param condition The condition to be checked.
 * @param sender The sender of the error message.
 * @param message The error message to be printed.
 * @param exitStatus The exit status of the program if the condition is false.
 */
#define ASSERT(condition, sender, message, exitStatus) \
  if (!(condition)) {                                  \
    process_safe_write(2, "%s %e%s", sender, message); \
    exit(exitStatus);                                  \
  }

/**
 * @file macros.h
 * @brief Contains a macro for asserting a condition and performing a goto statement if the condition is false.
 *
 * The ASSERT_GOTO macro is used to check a condition and perform a goto statement if the condition is false.
 * It takes four arguments: condition, sender, message, and label.
 *
 * @param condition The condition to be checked.
 * @param sender The sender of the error message.
 * @param message The error message to be printed.
 * @param label The label to jump to if the condition is false.
 */
#define ASSERT_GOTO(condition, sender, message, label) \
  if (!(condition)) {                                  \
    process_safe_write(2, "%s %e%s", sender, message); \
    goto label;                                        \
  }

#endif /* INC_MACROS */
