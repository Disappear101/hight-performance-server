cmake_minimum_required(VERSION 3.0)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

project(sylar)

include_directories(.)
include_directories(/usr/local/include)
#include_directories(/apps/sylar/include)

link_directories(/usr/local/lib)
#link_directories(/apps/sylar/lib)

find_library(YAMLCPP yaml-cpp)

set(LIB_SRC
    src/log.cpp
    src/util.cpp
    src/config.cpp
)

add_library(sylar SHARED ${LIB_SRC})

add_executable(test_log tests/test_log.cpp)
add_dependencies(test_log sylar)
target_link_libraries(test_log sylar ${YAMLCPP})

add_executable(test_config tests/test_config.cpp)
add_dependencies(test_config sylar)
target_link_libraries(test_config sylar ${YAMLCPP})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)