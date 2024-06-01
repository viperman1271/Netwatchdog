cmake_minimum_required(VERSION 3.10)

project(NetWatchdog)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(LIBBCRYPT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/libbcrypt/include)

if (NOT EXISTS ${LIBBCRYPT_INCLUDE_DIR}/bcrypt/BCrypt.hpp)
    message(FATAL_ERROR "libbcrypt not found at ${LIBBCRYPT_INCLUDE_DIR} [${LIBBCRYPT_INCLUDE_DIR}/bcrypt/BCrypt.hpp]")
endif()

include_directories(${CMAKE_SOURCE_DIR}/source/client)
include_directories(${CMAKE_SOURCE_DIR}/source/server)
include_directories(${LIBBCRYPT_INCLUDE_DIR})

# Set the output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
foreach(configuration_type IN ITEMS Debug Release)
    foreach(architecture IN ITEMS x86_64 x86)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${configuration_type} ${CMAKE_BINARY_DIR}/bin/${configuration_type}/${architecture})
    endforeach()
endforeach()

find_package(bsoncxx CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(cereal CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(cppzmq CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)
find_package(mongoc-1.0 CONFIG REQUIRED)
find_package(mongocxx CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(stduuid CONFIG REQUIRED)
find_package(toml11 CONFIG REQUIRED)
find_package(ZeroMQ CONFIG REQUIRED)

# Enable testing
enable_testing()
include(CTest)
include(Catch)

add_subdirectory("extern/libbcrypt")

# Common library
set(NETWATCHDOG_COMMON_SRC_DIR ${CMAKE_SOURCE_DIR}/source/common)
set(NETWATCHDOG_DATABASE_SRC_DIR ${CMAKE_SOURCE_DIR}/source/database)

set(BSONCXX_LIB $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>)
set(MONGOC_LIB $<IF:$<TARGET_EXISTS:mongo::mongoc_shared>,mongo::mongoc_shared,mongo::mongoc_static>)
set(MONGOCXXLIB $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>)

file(GLOB LIBNETWATCHDOG_SOURCES ${NETWATCHDOG_COMMON_SRC_DIR}/*.*)
file(GLOB NETWATCHDOG_DATABASE_SOURCES ${NETWATCHDOG_DATABASE_SRC_DIR}/*.*)

add_library(libnetwatchdog ${LIBNETWATCHDOG_SOURCES} ${NETWATCHDOG_DATABASE_SOURCES})
target_link_libraries(libnetwatchdog CLI11::CLI11 ${BSONCXX_LIB} ${MONGOC_LIB} ${MONGOCXXLIB} bcrypt stduuid)

include_directories(${NETWATCHDOG_DATABASE_SRC_DIR})

# Common Library Tests
set(NETWATCHDOG_COMMON_TEST_SRC_DIR ${CMAKE_SOURCE_DIR}/source/tests/common)
file(GLOB LIBNETWATCHDOG_TEST_SOURCES ${NETWATCHDOG_COMMON_TEST_SRC_DIR}/*.*)
add_executable(libwatchdog_tests ${LIBNETWATCHDOG_TEST_SOURCES})
target_link_libraries(libwatchdog_tests PRIVATE Catch2::Catch2WithMain libnetwatchdog libzmq-static)
target_compile_definitions(libwatchdog_tests PRIVATE NETWATCHDOG_CLIENT)

# Client executable
set(NETWATCHDOGC_SRC_DIR ${CMAKE_SOURCE_DIR}/source/client)
add_executable(netwatchdogc ${NETWATCHDOGC_SRC_DIR}/main.cpp ${NETWATCHDOGC_SRC_DIR}/client.h ${NETWATCHDOGC_SRC_DIR}/client.cpp)
target_link_libraries(netwatchdogc libnetwatchdog cppzmq-staic libzmq-static)
target_compile_definitions(netwatchdogc PRIVATE NETWATCHDOG_CLIENT ZMQ_HAVE_CURVE)

# Server executable
set(NETWATCHDOGD_SRC_DIR ${CMAKE_SOURCE_DIR}/source/server)
include_directories(${NETWATCHDOGD_SRC_DIR} ${NETWATCHDOG_COMMON_SRC_DIR})

file(GLOB NETWATCHDOGD_SOURCES ${NETWATCHDOGD_SRC_DIR}/*.*)

add_executable(netwatchdogd ${NETWATCHDOGD_SOURCES})
target_link_libraries(netwatchdogd PRIVATE libnetwatchdog cereal::cereal cppzmq-static libzmq-static toml11::toml11)
target_compile_definitions(netwatchdogd PRIVATE NETWATCHDOG_SERVER ZMQ_HAVE_CURVE)

# Web Lib
set(NETWATCHDOG_LIBWEB_SRC_DIR ${CMAKE_SOURCE_DIR}/source/libweb)

file(GLOB NETWATCHDOG_LIBWEB_SOURCES ${NETWATCHDOG_LIBWEB_SRC_DIR}/*.*)

add_library(libweb ${NETWATCHDOG_LIBWEB_SOURCES})
target_link_libraries(libweb libnetwatchdog CLI11::CLI11 cereal::cereal httplib::httplib nlohmann_json::nlohmann_json OpenSSL::SSL OpenSSL::Crypto ${BSONCXX_LIB} ${MONGOC_LIB} ${MONGOCXXLIB})
target_compile_definitions(libweb PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

include_directories(${NETWATCHDOG_DATABASE_SRC_DIR})

# Web server
set(NETWATCHDOG_WEB_SRC_DIR ${CMAKE_SOURCE_DIR}/source/web)

include_directories(${NETWATCHDOG_WEB_SRC_DIR} ${NETWATCHDOG_LIBWEB_SRC_DIR})

file(GLOB NETWATCHDOG_WEB_SOURCES ${NETWATCHDOG_WEB_SRC_DIR}/*.*)

add_executable(web ${NETWATCHDOG_WEB_SOURCES} ${NETWATCHDOG_LIBWEB_SRC_DIR}/applink.cpp)
target_compile_definitions(web PRIVATE NETWATCHDOG_SERVER)
target_link_libraries(web PRIVATE libnetwatchdog libweb)
target_compile_definitions(web PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

# Web Server Tests
set(NETWATCHDOG_WEB_TEST_SOURCE_DIR ${CMAKE_SOURCE_DIR}/source/tests/web)
file(GLOB NETWATCHDOG_WEB_TEST_SOURCES ${NETWATCHDOG_WEB_TEST_SOURCE_DIR}/*.* )
add_executable(web_tests ${NETWATCHDOG_WEB_TEST_SOURCES} ${NETWATCHDOG_LIBWEB_SRC_DIR}/applink.cpp)
target_link_libraries(web_tests PRIVATE Catch2::Catch2WithMain libnetwatchdog libweb)

# Admin executable
set(NETWATCHDOG_ADMIN_SRC_DIR ${CMAKE_SOURCE_DIR}/source/admin)

include_directories(${NETWATCHDOG_ADMIN_SRC_DIR})

file(GLOB NETWATCHDOG_ADMIN_SOURCES ${NETWATCHDOG_ADMIN_SRC_DIR}/*.*)

add_executable(admin ${NETWATCHDOG_ADMIN_SOURCES})
target_link_libraries(admin PRIVATE libnetwatchdog httplib::httplib nlohmann_json::nlohmann_json)

# Visual Studio directories
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_target_properties(libnetwatchdog PROPERTIES FOLDER "netwatchdog/libs")
set_target_properties(libweb PROPERTIES FOLDER "netwatchdog/libs")
set_target_properties(netwatchdogc PROPERTIES FOLDER "netwatchdog")
set_target_properties(netwatchdogd PROPERTIES FOLDER "netwatchdog")
set_target_properties(web PROPERTIES FOLDER "netwatchdog")
set_target_properties(admin PROPERTIES FOLDER "netwatchdog")
set_target_properties(libwatchdog_tests PROPERTIES FOLDER "netwatchdog/tests")
set_target_properties(web_tests PROPERTIES FOLDER "netwatchdog/tests")

set_target_properties(bcrypt PROPERTIES FOLDER "extern")
set_target_properties(bcrypt_test PROPERTIES FOLDER "extern/tests")

# Add tests
catch_discover_tests(libwatchdog_tests)
catch_discover_tests(web_tests)