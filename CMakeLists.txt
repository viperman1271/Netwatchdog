cmake_minimum_required(VERSION 3.10)

project(NetWatchdog)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include cppzmq submodule
set(CPPZMQ_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/cppzmq)

if (NOT EXISTS ${CPPZMQ_INCLUDE_DIR}/zmq.hpp)
    message(FATAL_ERROR "cppzmq not found at ${CPPZMQ_INCLUDE_DIR}")
endif()

include_directories(${CPPZMQ_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/source/lib)
include_directories(${CMAKE_SOURCE_DIR}/source/client)
include_directories(${CMAKE_SOURCE_DIR}/source/server)

# Set the output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
foreach(configuration_type IN ITEMS Debug Release)
    foreach(architecture IN ITEMS x86_64 x86)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${configuration_type} ${CMAKE_BINARY_DIR}/bin/${configuration_type}/${architecture})
    endforeach()
endforeach()

# Common library
add_library(common_lib source/common/netwatchdog.cpp)

# Client executable
add_executable(netwatchdogc ${CMAKE_SOURCE_DIR}/source/client/main.cpp)
target_link_libraries(netwatchdogc common_lib)

# Server executable
set(NETWATCHDOGD_SRC_DIR ${CMAKE_SOURCE_DIR}/source/server)
include_directories(${CMAKE_SOURCE_DIR}/source/server)
add_executable(netwatchdogd ${NETWATCHDOGD_SRC_DIR}/main.cpp ${NETWATCHDOGD_SRC_DIR}/server.h ${NETWATCHDOGD_SRC_DIR}/server.cpp ${NETWATCHDOGD_SRC_DIR}/monitor.h ${NETWATCHDOGD_SRC_DIR}/monitor.cpp)
target_link_libraries(netwatchdogd ${ZeroMQ_LIBRARIES} common_lib)

#message(STATUS "ZeroMQ include dir: ${ZeroMQ_INCLUDE_DIRS}")
#message(STATUS "ZeroMQ libraries: ${ZeroMQ_LIBRARIES}")