cmake_minimum_required(VERSION 3.0)
project(project_name)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/hight-performance-server)

include (hight-performance-server/cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++17 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

include_directories(.)
include_directories(/apps/tao/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hight-performance-server)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sylar/thirdpart)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sylar/thirdpart)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sylar/thirdpart/yaml-cpp/include)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sylar/thirdpart/jsoncpp-1.8.4/include)
link_directories(/apps/tao/lib)

find_package(Boost REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

#find_package(Protobuf REQUIRED)
#if(Protobuf_FOUND)
#    include_directories(${Protobuf_INCLUDE_DIRS})
#endif()
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

set(LIB_SRC
        template/my_module.cpp
    )

#PROTOBUF_GENERATE_CPP(PB_SRCS PB_HDRS sylar/test.proto)
#list(APPEND LIB_SRC ${PB_SRCS})
#
#message(STATUS ${LIB_SRC})

add_library(project_name SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(project_name)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
