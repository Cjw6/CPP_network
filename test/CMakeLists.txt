include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/3rd/googletest/googletest/include)

add_executable(run_test
RunTest.cpp
TestAnyVoidPointer.cpp
TestTime.cpp
TestReadH264.cpp
)

target_link_libraries(run_test easycpp_util easycpp_media easycpp_network gtest)

