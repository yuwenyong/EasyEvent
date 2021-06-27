add_executable(EchoServerSecure EchoServerSecure.cpp)
add_dependencies(EchoServerSecure EasyEvent)
target_link_libraries(EchoServerSecure EasyEvent)
add_custom_command(TARGET EchoServerSecure
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/test.crt ${CMAKE_CURRENT_BINARY_DIR}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/test.key ${CMAKE_CURRENT_BINARY_DIR}/
)