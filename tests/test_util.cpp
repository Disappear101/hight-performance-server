#include "../src/log.h"
#include "../src/util.h"
#include <assert.h>
#include <string>
#include "../src/macro.h"

tao::Logger::ptr g_logger = TAO_LOG_ROOT();

void test_assert() {
    TAO_LOG_INFO(g_logger) << tao::BacktraceToString(10);
    TAO_ASSERT(false);
}

int main(int argc, char** argv) {
    test_assert();
    return 0;
}