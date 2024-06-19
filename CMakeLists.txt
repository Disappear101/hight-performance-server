cmake_minimum_required(VERSION 3.0)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -g -std=c++17 -Wall -Wno-deprecated -Werror -Wno-unused-function")

project(tao)

include_directories(.)
include_directories(/usr/local/include)
#include_directories(/apps/tao/include)

link_directories(/usr/local/lib)
#link_directories(/apps/tao/lib)

find_library(YAMLCPP yaml-cpp)
find_library(PTHREAD pthread)

set(LIB_SRC
    src/address.cpp
    src/log.cpp
    src/util.cpp
    src/config.cpp
    src/mutex.cpp
    src/thread.cpp
    src/fiber.cpp
    src/scheduler.cpp
    src/timer.cpp
    src/fdmanager.cpp
    src/iomanager.cpp
    src/hook.cpp
)

add_library(tao SHARED ${LIB_SRC})

set(LIB_LIB
    tao
    pthread
    ${YAMLCPP}
    dl
    )

add_executable(test_log tests/test_log.cpp)
add_dependencies(test_log tao)
target_link_libraries(test_log ${LIB_LIB})

add_executable(test_config tests/test_config.cpp)
add_dependencies(test_config tao)
target_link_libraries(test_config ${LIB_LIB})

add_executable(test_thread tests/test_thread.cpp)
add_dependencies(test_thread tao)
target_link_libraries(test_thread ${LIB_LIB})

add_executable(test_util tests/test_util.cpp)
add_dependencies(test_util tao)
target_link_libraries(test_util ${LIB_LIB})

add_executable(test_fiber tests/test_fiber.cpp)
add_dependencies(test_fiber tao)
target_link_libraries(test_fiber ${LIB_LIB})

add_executable(test_scheduler tests/test_scheduler.cpp)
add_dependencies(test_scheduler tao)
target_link_libraries(test_scheduler ${LIB_LIB})

add_executable(test_iomanager tests/test_iomanager.cpp)
add_dependencies(test_iomanager tao)
target_link_libraries(test_iomanager ${LIB_LIB})

add_executable(test_hook tests/test_hook.cpp)
add_dependencies(test_hook tao)
target_link_libraries(test_hook ${LIB_LIB})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)