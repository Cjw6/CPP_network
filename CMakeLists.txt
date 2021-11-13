cmake_minimum_required(VERSION 3.10.0)
project(ServerProject VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

option(WITH_ASAN "" ON)

if(WITH_ASAN)
message("using Address Sanitizer")
set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address -fno-omit-frame-pointer ")
endif()

add_subdirectory(src)