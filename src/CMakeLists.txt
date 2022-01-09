include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd)

# add_subdirectory(3rd/threadpool11)

# add_library(easycpp_util SHARED  
# util/Env.h
# util/FastRand.cpp
# util/FastRand.h
# util/LockFreeMPQueue.hpp
# util/NonCopy.h
# util/SafeFree.h
# util/SafeQueue.h
# util/Singleton.h
# util/Thread.cpp
# util/Thread.h
# util/ThreadPool.h
# util/TimeCost.h)

# target_link_libraries(easycpp_util )


add_library(easycpp_network SHARED 
network/Acceptor.cpp
network/Acceptor.h
network/Buffer.cpp
network/Buffer.h
network/Channels.cpp
network/Channels.h
network/ConnectManager.cpp
network/ConnectManager.h
network/Dispatcher.cpp
network/Dispatcher.h
network/epoll.c
network/Socket.cpp
network/Socket.h
network/TcpConnect.cpp
network/TcpConnect.h
network/TcpServer.cpp
network/TcpServer.h
protocol
protocol/adler32.cpp
protocol/adler32.h
protocol/EncodeInt.cpp
protocol/EncodeInt.h
protocol/MsgBuf.cpp
protocol/MsgBuf.h
protocol/MsgBufSlice.cpp
protocol/MsgBufSlice.h
protocol/MsgCodec.cpp
protocol/MsgCodec.h
protocol/MsgCodecType.h
protocol/MsgFixedBlock.cpp
protocol/MsgFixedBlock.h
protocol/MsgParse.cpp
protocol/MsgParse.h
util
util/Env.h
util/FastRand.cpp
util/FastRand.h
util/LockFreeMPQueue.hpp
util/Log.h
util/NonCopy.h
util/SafeFree.h
util/SafeQueue.h
util/Singleton.h
util/Thread.cpp
util/Thread.h
util/ThreadPool.h
util/TimeCost.h
)

target_link_libraries(easycpp_network  glog pthread)


add_executable(simple_server example/simple_server.cpp)
target_link_libraries(simple_server easycpp_network)


add_executable(simple_client example/simple_client.cpp)
target_link_libraries(simple_client easycpp_network)
