CollectSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} PRIVATE_SOURCES)

add_definitions(-DEASY_EVENT_API_EXPORT)

if (BUILD_EasyEvent_AS_STATIC_LIB)
    add_library(EasyEvent STATIC ${PRIVATE_SOURCES})
else()
    add_library(EasyEvent SHARED ${PRIVATE_SOURCES})
endif()

target_include_directories(EasyEvent PUBLIC "${CMAKE_SOURCE_DIR}/src/")
