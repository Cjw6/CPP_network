include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(easycpp_util SHARED  
util/Thread.h
util/Thread.cpp)
target_link_libraries(easycpp_util glog pthread)


add_library(easycpp_network SHARED 
network/Acceptor.cpp
network/Acceptor.h
network/Channels.cpp
network/Channels.h
network/Dispatcher.cpp
network/Dispatcher.h
network/Socket.cpp
network/Socket.h
network/TcpConnect.h
network/TcpConnect.cpp
network/TcpServer.h
network/TcpServer.cpp)
target_link_libraries(easycpp_network  easycpp_util)

add_executable(simple_server example/simple_server.cpp)
target_link_libraries(simple_server easycpp_network)

add_executable(simple_client example/simple_client.cpp)
target_link_libraries(simple_client easycpp_network)
