CollectSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} PRIVATE_SOURCES)

add_definitions(-DEASY_EVENT_API_EXPORT)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if (BUILD_EasyEvent_AS_STATIC_LIB)
    add_library(EasyEvent STATIC ${PRIVATE_SOURCES})
    target_link_libraries(EasyEvent INTERFACE openssl Threads::Threads)
else()
    add_library(EasyEvent SHARED ${PRIVATE_SOURCES})
    target_link_libraries(EasyEvent PUBLIC openssl Threads::Threads)
endif()

target_include_directories(EasyEvent PUBLIC ${OPENSSL_INCLUDE_DIR} "${CMAKE_SOURCE_DIR}/src/")
