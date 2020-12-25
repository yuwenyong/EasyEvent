
add_executable(EchoClient EchoClient.cpp)
add_dependencies(EchoClient EasyEvent)
target_link_libraries(EchoClient EasyEvent)