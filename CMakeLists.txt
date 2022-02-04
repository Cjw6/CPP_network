cmake_minimum_required(VERSION 3.10.0)
project(ServerProject VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

option(WITH_ASAN "" ON)

if(WITH_ASAN)
message("using Address Sanitizer")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif()
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC")

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(3rd/googletest)
# add_subdirectory(3rd/fmt)