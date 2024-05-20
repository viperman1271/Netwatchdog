cmake_minimum_required(VERSION 3.10)

project(NetWatchdog)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${CMAKE_SOURCE_DIR}/source/client)
include_directories(${CMAKE_SOURCE_DIR}/source/server)

# Set the output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
foreach(configuration_type IN ITEMS Debug Release)
    foreach(architecture IN ITEMS x86_64 x86)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${configuration_type} ${CMAKE_BINARY_DIR}/bin/${configuration_type}/${architecture})
    endforeach()
endforeach()

find_package(cereal CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(cppzmq CONFIG REQUIRED)
find_package(mongoc-1.0 CONFIG REQUIRED)
find_package(bsoncxx CONFIG REQUIRED)
find_package(mongocxx CONFIG REQUIRED)
find_package(stduuid CONFIG REQUIRED)
find_package(toml11 CONFIG REQUIRED)

# Common library
set(NETWATCHDOG_COMMON_SRC_DIR ${CMAKE_SOURCE_DIR}/source/common)
file(GLOB LIBNETWATCHDOG_SOURCES ${NETWATCHDOG_COMMON_SRC_DIR}/*.*)

# Client executable
set(NETWATCHDOGC_SRC_DIR ${CMAKE_SOURCE_DIR}/source/client)
add_executable(netwatchdogc ${NETWATCHDOGC_SRC_DIR}/main.cpp ${NETWATCHDOGC_SRC_DIR}/client.h ${NETWATCHDOGC_SRC_DIR}/client.cpp ${LIBNETWATCHDOG_SOURCES})
target_link_libraries(netwatchdogc CLI11::CLI11 cppzmq)
target_compile_definitions(netwatchdogc PRIVATE NETWATCHDOG_CLIENT)

# Server executable
set(NETWATCHDOGD_SRC_DIR ${CMAKE_SOURCE_DIR}/source/server)
set(BSONCXX_LIB $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>)
set(MONGOC_LIB $<IF:$<TARGET_EXISTS:mongo::mongoc_shared>,mongo::mongoc_shared,mongo::mongoc_static>)
set(MONGOCXXLIB $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>)

include_directories(${NETWATCHDOGD_SRC_DIR} ${NETWATCHDOG_COMMON_SRC_DIR})

file(GLOB NETWATCHDOGD_SOURCES ${NETWATCHDOGD_SRC_DIR}/*.*)
file(GLOB NETWATCHDOGD_DATABASE_SOURCES ${NETWATCHDOGD_SRC_DIR}/database/*.*)

add_executable(netwatchdogd ${NETWATCHDOGD_SOURCES} ${NETWATCHDOGD_DATABASE_SOURCES} ${LIBNETWATCHDOG_SOURCES})
target_link_libraries(netwatchdogd PRIVATE cereal::cereal CLI11::CLI11 cppzmq ${BSONCXX_LIB} ${MONGOC_LIB} ${MONGOCXXLIB} stduuid toml11::toml11)
target_compile_definitions(netwatchdogd PRIVATE NETWATCHDOG_SERVER)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_target_properties(netwatchdogc PROPERTIES FOLDER "NetWatchdog")
set_target_properties(netwatchdogd PROPERTIES FOLDER "NetWatchdog")