include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/fmt/include)

#util
add_library(easycpp_util SHARED  
util/Env.h
util/FastRand.cpp
util/FastRand.h
util/LockFreeMPQueue.hpp
util/Log.h
util/NonCopy.h
util/SafeMem.h
util/SafeQueue.h
util/Singleton.h
util/Thread.cpp
util/Thread.h
util/thrdpool.c
util/thrdpool.h
util/ThreadPoolWrap.h
util/list.h
util/TimeCost.h
util/StringSplit.cpp
util/ByteBuffer.h
util/ByteBuffer.cpp
util/TimerUs.cpp
util/TimerUs.h
util/Adler32.cpp
util/Adler32.h
util/File.h
util/File.cpp
util/Skiplist.h
util/Skiplist.cpp
util/thrdpool.c
)
target_link_libraries(easycpp_util glog gflags pthread)

add_library(easycpp_protocol SHARED 
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
)

#network
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
network/Socket.cpp
network/Socket.h
network/TcpConnect.cpp
network/TcpConnect.h
network/TcpServer.cpp
network/TcpServer.h
network/WakeUp.cpp
network/WakeUp.h
)
target_link_libraries(easycpp_network easycpp_util)

#media
add_library(easycpp_media SHARED 
media/MediaSession.h
media/MediaSession.cpp

media/RtspServer.cpp
media/RtspServer.h
media/RtspSession.cpp
media/RtspSession.h
media/RtpPacket.h

media/RtpSource.h
media/RtpSource.cpp
media/RtpSession.h
media/RtpSession.cpp

media/ReadH264File.cpp
media/ReadH264File.h
media/ReadAACFile.h
media/ReadAACFile.cpp
)
target_link_libraries(easycpp_media easycpp_network easycpp_util)

#simple server
add_executable(simple_server example/simple_server.cpp)
target_link_libraries(simple_server easycpp_network)

#simple_client
add_executable(simple_client example/simple_client.cpp)
target_link_libraries(simple_client easycpp_network)

#rtsp server
add_executable(simple_rtsp_server example/simple_rtspserver.cpp)
target_link_libraries(simple_rtsp_server easycpp_media)

#read h264
add_executable(read_h264 example/read_h264.cpp)
target_link_libraries(read_h264 easycpp_media)

#example_timer
add_executable(example_timer example/example_timer.cpp)
target_link_libraries(example_timer easycpp_network)

#xample_wakeup
add_executable(example_wakeup example/example_wakeup.cpp)
target_link_libraries(example_wakeup easycpp_network)