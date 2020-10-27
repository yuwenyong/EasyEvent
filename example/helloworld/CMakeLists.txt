
add_executable(helloworld helloworld.cpp)
add_dependencies(helloworld EasyEvent)
target_link_libraries(helloworld EasyEvent)