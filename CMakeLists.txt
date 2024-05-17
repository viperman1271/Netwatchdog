cmake_minimum_required(VERSION 3.10)

project(NetWatchdog)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_path(ZeroMQ_INCLUDE_DIR zmq.h)
find_library(ZeroMQ_LIBRARY NAMES zmq libzmq)

if (ZeroMQ_INCLUDE_DIR AND ZeroMQ_LIBRARY)
    set(ZeroMQ_LIBRARIES ${ZeroMQ_LIBRARY})
    set(ZeroMQ_INCLUDE_DIRS ${ZeroMQ_INCLUDE_DIR})
else()
    message(FATAL_ERROR "ZeroMQ not found")
endif()

include_directories(${ZeroMQ_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/source/lib)

# Set the output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
foreach(configuration_type IN ITEMS Debug Release)
    foreach(architecture IN ITEMS x86_64 x86)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${configuration_type} ${CMAKE_BINARY_DIR}/bin/${configuration_type}/${architecture})
    endforeach()
endforeach()

# Common library
add_library(common_lib source/lib/netwatchdog.cpp)

# Client executable
include_directories(${CMAKE_SOURCE_DIR}/source/client)
add_executable(netwatchdogc source/client/main.cpp)
target_link_libraries(netwatchdogc ${ZeroMQ_LIBRARIES} common_lib)

# Server executable
include_directories(${CMAKE_SOURCE_DIR}/source/server)
add_executable(netwatchdogd source/server/main.cpp)
target_link_libraries(netwatchdogd ${ZeroMQ_LIBRARIES} common_lib)

message(STATUS "ZeroMQ include dir: ${ZeroMQ_INCLUDE_DIRS}")
message(STATUS "ZeroMQ libraries: ${ZeroMQ_LIBRARIES}")