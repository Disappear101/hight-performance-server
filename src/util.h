#ifndef __TAO_UTIL_H__
#define __TAO_UTIL_H__

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>

namespace tao {

/**
 * @brief return current thread id
 */
pid_t GetThreadId();

/**
 * @brief return current coroutine id
 */
uint32_t GetFiberId();

void Backtrace(std::vector<std::string>& bt, int size, int skip = 1);

std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");


}

#endif