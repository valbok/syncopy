project(syncopy)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig)
pkg_check_modules(OPENSSL REQUIRED openssl)

include_directories(./syncopy)

add_subdirectory(lib/rpclib)
add_subdirectory(syncopy)

include_directories(./lib/rpclib/include)
include_directories(./)

add_subdirectory(bin)

include_directories(lib/googletest/googletest)
include_directories(lib/googletest/googletest/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
add_library(gtest
    lib/googletest/googletest/src/gtest_main.cc
    lib/googletest/googletest/src/gtest-all.cc)


add_subdirectory(tests)
