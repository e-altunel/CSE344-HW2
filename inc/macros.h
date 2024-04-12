#ifndef INC_MACROS
#define INC_MACROS

#include <stdlib.h>
#include <write.h>

#define ASSERT(condition, message, exitStatus) \
  if (!(condition)) {                          \
    process_safe_write(2, message);            \
    exit(exitStatus);                          \
  }
#define ASSERT_GOTO(condition, message, label) \
  if (!(condition)) {                          \
    process_safe_write(2, message);            \
    goto label;                                \
  }

#endif /* INC_MACROS */
