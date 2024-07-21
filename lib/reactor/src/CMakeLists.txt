
add_library(reactor-static STATIC 
Acceptor.cc
Condition.cc
EchoServer.cc
EventLoop.cc
InetAddress.cc
MutexLock.cc
Socket.cc
SocketIO.cc
TaskQueue.cc
TcpConnection.cc
TcpServer.cc
Thread.cc
ThreadPool.cc
                            )

target_include_directories(reactor-static PUBLIC ${CMAKE_SOURCE_DIR}/header_only_include)
target_include_directories(reactor-static PUBLIC ../include)
target_link_libraries(reactor-static PUBLIC dictProducer-static)
target_link_libraries(reactor-static PUBLIC PageLibPreprocessor-static)
target_link_libraries(reactor-static PRIVATE re2::re2)
target_link_libraries(reactor-static PRIVATE spdlog::spdlog)
target_link_libraries(reactor-static PRIVATE Cache_manager-static)