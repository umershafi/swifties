cmake_minimum_required(VERSION 3.10.0)
project(swifties)

# Make sure developers do not run cmake in the main project directory, to keep
# build artifacts from becoming clutter
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()

# Turn on debug builds if we are building in a devel shell
if (CMAKE_BUILD_TYPE STREQUAL "" AND "$ENV{DEVEL_SHELL}" STREQUAL "1")
    message(STATUS "Setting debug build type by default in devel shell")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Output binaries to a sub directory "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable cmake testing
include(CTest)
enable_testing()

# Enable GoogleTest
include(GoogleTest)
add_subdirectory(/usr/src/googletest googletest)

# Enable Boost
# Use static libraries so binaries can be deployed without a full boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system filesystem log log_setup regex)
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(include)

# Locate bash program
find_program (BASH_PROGRAM bash)

add_library(logger src/logger.cc)
add_library(session src/session.cc src/server.cc)
add_library(server_c src/server.cc src/session.cc)
add_library(config_parser src/config_parser.cc)
add_library(request_parser src/http/request_parser.cc)
add_library(request_handler_dispatcher src/request_handler_dispatcher.cc)
add_library(file_storage src/api/file_storage.cc) 
add_library(crud_handler src/api/crud_handler.cc)
add_library(request_handler
            src/request_handler_dispatcher.cc
            src/request_handler/request_handler_static.cc
            src/request_handler/request_handler_echo.cc
            src/request_handler/request_handler_404.cc
            src/request_handler/request_handler_api.cc
            src/request_handler/request_handler_health.cc
            src/request_handler/request_handler_sleep.cc
            src/http/mime_types.cc)

add_executable(server src/server_main.cc)
target_link_libraries(server logger server_c session request_handler request_parser request_handler_dispatcher
                      config_parser file_storage crud_handler Boost::system Boost::filesystem 
                      Boost::regex Boost::log_setup  Boost::log) 

add_custom_target(force_rebuild ALL
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_SOURCE_DIR}/src/session.cc
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_SOURCE_DIR}/src/server.cc
)

add_dependencies(server force_rebuild)

# Test executables
add_executable(config_parser_test tests/config_parser_test.cc)
add_executable(server_test tests/server_test.cc)
add_executable(session_test tests/session_test.cc)
add_executable(request_parser_test tests/request_parser_test.cc)
add_executable(request_handler_test tests/request_handler_test.cc)
add_executable(request_handler_dispatcher_test tests/request_handler_dispatcher_test.cc)
add_executable(logger_test tests/logger_test.cc)
add_executable(file_storage_test tests/file_storage_test.cc) 
add_executable(crud_handler_test tests/crud_handler_test.cc)
target_link_libraries(crud_handler file_storage Boost::filesystem)
target_link_libraries(config_parser_test config_parser gtest_main)
target_link_libraries(file_storage_test file_storage gtest_main Boost::filesystem)
target_link_libraries(crud_handler_test crud_handler gtest_main Boost::filesystem) 
target_link_libraries(server_test session request_parser config_parser request_handler logger file_storage crud_handler Boost::log_setup Boost::log Boost::system Boost::regex gtest_main gmock_main)
target_link_libraries(session_test session config_parser request_parser request_handler logger file_storage crud_handler Boost::log_setup Boost::log Boost::system Boost::filesystem Boost::regex gtest_main gmock_main)
target_link_libraries(request_parser_test request_parser file_storage crud_handler gtest_main Boost::system)
target_link_libraries(request_handler_test request_parser request_handler file_storage crud_handler gtest_main gmock_main Boost::system  Boost::filesystem logger Boost::log_setup Boost::log)
target_link_libraries(request_handler_dispatcher_test request_parser config_parser request_handler request_handler_dispatcher file_storage crud_handler gtest_main gmock_main Boost::system  Boost::filesystem logger Boost::log_setup Boost::log)
target_link_libraries(logger_test logger gtest_main Boost::system Boost::log_setup Boost::log)

gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(server_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(session_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(request_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(request_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(request_handler_dispatcher_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(logger_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(file_storage_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests) 
gtest_discover_tests(crud_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests) 
add_test(NAME integration_test COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration_test.sh ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server)

include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(TARGETS config_parser server session request_parser request_handler request_handler_dispatcher logger file_storage crud_handler TESTS config_parser_test server_test session_test request_parser_test request_handler_test request_handler_dispatcher_test logger_test file_storage_test crud_handler_test)
