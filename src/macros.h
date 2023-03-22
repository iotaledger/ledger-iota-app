#ifndef MACROS_H
#define MACROS_H

#include "os.h"

// throws if expression is not true
#define MUST_THROW(c)                                                          \
    {                                                                          \
        if (!(c)) {                                                            \
            THROW(SW_UNKNOWN);                                                 \
        }                                                                      \
    }

// return 0 if expression is not true
#define MUST(c)                                                                \
    {                                                                          \
        if (!(c)) {                                                            \
            return 0;                                                          \
        }                                                                      \
    }

// wrap-around safe check for addition
#define MUST_SUM_LOWER_THAN(a, b, sum)                                         \
    {                                                                          \
        if (!((a < sum) && (b < sum) && ((a + b) < sum))) {                    \
            return 0;                                                          \
        }                                                                      \
    }

/// Throws provided error, if condition does not apply
#define VALIDATE(cond, error)                                                  \
    {                                                                          \
        if (!(cond)) {                                                         \
            THROW(error);                                                      \
        }                                                                      \
    }

//#define UNUSED __attribute__((unused))

#endif // MACROS_H
