
add_executable(EchoServer EchoServer.cpp)
add_dependencies(EchoServer EasyEvent)
target_link_libraries(EchoServer EasyEvent)