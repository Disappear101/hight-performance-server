cmake_minimum_required(VERSION 3.0)

project(tao)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include (cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -g -std=c++17 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++17 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

include_directories(.)
include_directories(/usr/local/include)
include_directories(/usr/include/openssl)
#include_directories(/apps/tao/include)
link_directories(/usr/local/lib)
#link_directories(/apps/tao/lib)

option(BUILD_TEST "ON for complile test" OFF)

find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIR})
endif()

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

find_library(YAMLCPP yaml-cpp)
find_library(PTHREAD pthread)

set(LIB_SRC
    src/address.cpp
    src/application.cpp
    src/daemon.cpp
    src/bytearray.cpp
    src/env.cpp
    src/http/http.cpp
    src/http/http_parser.cpp
    src/http/http_session.cpp
    src/http/http_server.cpp
    src/http/http_connection.cpp
    src/http/ws_session.cpp
    src/http/ws_server.cpp
    src/http/servlets/status_servlet.cpp
    src/http/ws_servlet.cpp
    src/http/servlet.cpp
    src/socket.cpp
    src/streams/socket_stream.cpp
    src/streams/zlib_stream.cpp
    src/stream.cpp
    src/utils/hash_util.cpp
    src/log.cpp
    src/util.cpp
    src/config.cpp
    src/mutex.cpp
    src/thread.cpp
    src/fiber.cpp
    src/scheduler.cpp
    src/tcpserver.cpp
    src/timer.cpp
    src/fdmanager.cpp
    src/iomanager.cpp
    src/hook.cpp
    src/worker.cpp
    src/module.cpp
    src/protocol.cpp
    src/library.cpp
)

ragelmaker(src/http/http11_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/http)
ragelmaker(src/http/httpclient_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/http)
ragelmaker(src/http/uri.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/http)

add_library(tao SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(tao)

set(LIB_LIB
    tao
    ${ZLIB_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    pthread
    ${YAMLCPP}
    dl
    )

if(BUILD_TEST)
tao_add_executable(test_log "tests/test_log.cpp" tao "${LIB_LIB}")
tao_add_executable(test_config "tests/test_config.cpp" tao "${LIB_LIB}")
tao_add_executable(test_thread "tests/test_thread.cpp" tao "${LIB_LIB}")
tao_add_executable(test_util "tests/test_util.cpp" tao "${LIB_LIB}")
tao_add_executable(test_fiber "tests/test_fiber.cpp" tao "${LIB_LIB}")
tao_add_executable(test_scheduler "tests/test_scheduler.cpp" tao "${LIB_LIB}")
tao_add_executable(test_iomanager "tests/test_iomanager.cpp" tao "${LIB_LIB}")
tao_add_executable(test_hook "tests/test_hook.cpp" tao "${LIB_LIB}")
tao_add_executable(test_address "tests/test_address.cpp" tao "${LIB_LIB}")
tao_add_executable(test_socket "tests/test_socket.cpp" tao "${LIB_LIB}")
tao_add_executable(test_bytearray "tests/test_bytearray.cpp" tao "${LIB_LIB}")
tao_add_executable(test_http "tests/test_http.cpp" tao "${LIB_LIB}")
tao_add_executable(test_http_parser "tests/test_http_parser.cpp" tao "${LIB_LIB}")
tao_add_executable(test_tcpserver "tests/test_tcpserver.cpp" tao "${LIB_LIB}")
tao_add_executable(test_fdmanager "tests/test_fdmanager.cpp" tao "${LIB_LIB}")
tao_add_executable(test_http_server "tests/test_http_server.cpp" tao "${LIB_LIB}")
tao_add_executable(echo_server "examples/echo_server.cpp" tao "${LIB_LIB}")
tao_add_executable(test_http_connection "tests/test_http_connection.cpp" tao "${LIB_LIB}")
tao_add_executable(test_uri "tests/test_uri.cpp" tao "${LIB_LIB}")
tao_add_executable(load_test "tests/load_test.cpp" tao "${LIB_LIB}")
tao_add_executable(test_daemon "tests/test_daemon.cpp" tao "${LIB_LIB}")
tao_add_executable(test_application "tests/test_application.cpp" tao "${LIB_LIB}")
tao_add_executable(test_ws_server "tests/test_ws_server.cpp" tao "${LIB_LIB}")
endif()

tao_add_executable(bin_tao "src/main.cpp" tao "${LIB_LIB}")
set_target_properties(bin_tao PROPERTIES OUTPUT_NAME "tao")


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)