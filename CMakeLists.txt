cmake_minimum_required(VERSION 3.22)

project(CmdLineParser VERSION 0.0.1 LANGUAGES CXX )

find_package(GTest CONFIG REQUIRED)
get_property(gtest_INCLUDE_DIR TARGET gtest::extra PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

add_executable(CmdLineParser
        CmdLineParser.cpp
        CmdLineParserTest.cpp
)

target_include_directories(CmdLineParser PUBLIC ${gtest_INCLUDE_DIR} })
target_link_libraries(CmdLineParser gtest::gtest)


