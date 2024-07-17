#ifndef __TAO_MACRO_H__
#define __TAO_MACRO_H__

#include <assert.h>
#include <string>
#include "util.h"
#include "log.h"

/*__builtin_expect(long exp, long c): It indicates that the expression exp
 is expected to be equal to c most of the time. This hint helps the compiler
 to optimize the code better by arranging branches in a way that is favorable 
 to the predicted outcome, potentially improving branch prediction on some 
 processors and allowing more efficient pipelining of instructions.*/
#if defined __GNUC__ || defined __llvm__
#   define TAO_LIKELY(x)    __builtin_expect(!!(x), 1)
#   define TAO_UNLIKELY(x)  __builtin_expect(!!(x), 0)
#else
#   define TAO_LIKELY(x)    (x)
#   define TAO_LIKELY(x)    (x)
#endif

#define TAO_ASSERT(x) \
    if (TAO_UNLIKELY(!(x))) { \
        TAO_LOG_ERROR(TAO_LOG_ROOT()) << "ASSERTION: " #x \
            << " \nbacktrace: \n" \
            << tao::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#define TAO_ASSERT2(x, w) \
    if (TAO_UNLIKELY(!(x))) { \
        TAO_LOG_ERROR(TAO_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w \
            << " \nbacktrace: \n" \
            << tao::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#endif