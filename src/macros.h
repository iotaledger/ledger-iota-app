#ifndef MACROS_H
#define MACROS_H

#include "os.h"


/// Throws provided error, if condition does not apply
#define VALIDATE(cond, error)                                                  \
    {                                                                          \
        if (!(cond)) {                                                         \
            THROW(error);                                                      \
        }                                                                      \
    }

//#define UNUSED __attribute__((unused))

#endif // MACROS_H
