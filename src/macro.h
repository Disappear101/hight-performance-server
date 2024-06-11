#ifndef __TAO_MACRO_H__
#define __TAO_MACRO_H__

#include <assert.h>
#include <string>
#include "util.h"

#define TAO_ASSERT(x) \
    if (!(x)) { \
        TAO_LOG_ERROR(TAO_LOG_ROOT()) << "ASSERTION: " #x \
            << " \nbacktrace: \n" \
            << tao::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#define TAO_ASSERT2(x, w) \
    if (!(x)) { \
        TAO_LOG_ERROR(TAO_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w \
            << " \nbacktrace: \n" \
            << tao::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#endif