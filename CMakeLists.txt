cmake_minimum_required(VERSION 3.10)

project(NetWatchdog)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include cppzmq submodule
set(CPPZMQ_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/cppzmq)
if (NOT EXISTS ${CPPZMQ_INCLUDE_DIR}/zmq.hpp)
    message(FATAL_ERROR "cppzmq not found at ${CPPZMQ_INCLUDE_DIR}")
endif()

set(TOML11_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/toml11)
if (NOT EXISTS ${TOML11_INCLUDE_DIR}/toml.hpp)
    message(FATAL_ERROR "toml11 not found at ${TOML11_INCLUDE_DIR}")
endif()

set(CLI11_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/cli11/include)
if (NOT EXISTS ${CLI11_INCLUDE_DIR}/CLI/CLI.hpp)
    message(FATAL_ERROR "cli11 not found at ${CLI11_INCLUDE_DIR}")
endif()

include_directories(${CPPZMQ_INCLUDE_DIR})
include_directories(${TOML11_INCLUDE_DIR})
include_directories(${CLI11_INCLUDE_DIR})
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
set(NETWATCHDOG_COMMON_SRC_DIR ${CMAKE_SOURCE_DIR}/source/common)
file(GLOB LIBNETWATCHDOG_SOURCES ${NETWATCHDOG_COMMON_SRC_DIR}/*.*)

# Client executable
set(NETWATCHDOGC_SRC_DIR ${CMAKE_SOURCE_DIR}/source/client)
add_executable(netwatchdogc ${NETWATCHDOGC_SRC_DIR}/main.cpp ${NETWATCHDOGC_SRC_DIR}/client.h ${NETWATCHDOGC_SRC_DIR}/client.cpp ${LIBNETWATCHDOG_SOURCES})
target_link_libraries(netwatchdogc)
target_compile_definitions(netwatchdogc PRIVATE NETWATCHDOG_CLIENT)

add_subdirectory(extern/mongo-c-driver)
add_subdirectory(extern/mongo-cxx-driver)
#add_subdirectory(extern/mongo-cxx-driver/src/bsoncxx)

# Find the mongocxx and bsoncxx packages
#find_package(mongocxx-static REQUIRED)
#find_package(bsoncxx-static REQUIRED)
#find_package(bson_static REQUIRED)
#find_package(bson)
#find_package(mongocxx CONFIG REQUIRED)

# Server executable
set(NETWATCHDOGD_SRC_DIR ${CMAKE_SOURCE_DIR}/source/server)
set(MONGOCXX_BUILD_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/build/extern/mongo-cxx-driver/src/mongocxx/lib/mongocxx/v_noabi)
set(MONGOCXX_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/mongo-cxx-driver/src/mongocxx/include/mongocxx/v_noabi)
set(BSONCXX_BUILD_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/build/extern/mongo-cxx-driver/src/bsoncxx/lib/bsoncxx/v_noabi)
set(BSONCXX_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/mongo-cxx-driver/src/bsoncxx/include/bsoncxx/v_noabi)

include_directories(${NETWATCHDOGD_SRC_DIR} ${NETWATCHDOG_COMMON_SRC_DIR} ${MONGOCXX_STATIC_INCLUDE_DIRS} ${BSONCXX_STATIC_INCLUDE_DIRS} ${MONGOCXX_BUILD_INCLUDE_DIR} ${MONGOCXX_INCLUDE_DIR} ${BSONCXX_BUILD_INCLUDE_DIR} ${BSONCXX_INCLUDE_DIR})
file(GLOB NETWATCHDOGD_SOURCES ${NETWATCHDOGD_SRC_DIR}/*.*)
file(GLOB NETWATCHDOGD_DATABASE_SOURCES ${NETWATCHDOGD_SRC_DIR}/database/*.*)
add_executable(netwatchdogd ${NETWATCHDOGD_SOURCES} ${NETWATCHDOGD_DATABASE_SOURCES} ${LIBNETWATCHDOG_SOURCES})
target_link_libraries(netwatchdogd ${ZeroMQ_LIBRARIES} bson_static mongoc_static)
target_compile_definitions(netwatchdogd PRIVATE NETWATCHDOG_SERVER)

set_target_properties(netwatchdogc PROPERTIES FOLDER "NetWatchdog")
set_target_properties(netwatchdogd PROPERTIES FOLDER "NetWatchdog")