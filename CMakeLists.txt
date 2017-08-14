cmake_minimum_required(VERSION 3.5.1)
project(monitor_prod)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

find_package (Boost REQUIRED COMPONENTS system filesystem program_options)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

find_package(Threads)
include_directories(SYSTEM ${Threads_INCLUDE_DIR})

file(GLOB CPPS "source/*.cpp")
file(GLOB HS "source/headers/*.h")

add_executable(${PROJECT_NAME} ${CPPS} ${HS})

target_link_libraries(${PROJECT_NAME} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})