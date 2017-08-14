project(monitor)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/../)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB CPPS "*.cpp")
#add_definitions(-Wall -O2)
find_package(PkgConfig REQUIRED)
pkg_check_modules(jsoncpp jsoncpp)

find_package(jsoncpp REQUIRED)
include_directories(${jsoncpp_INCLUDE_DIRS})

find_package(Threads)
include_directories(SYSTEM ${Threads_INCLUDE_DIR})

find_package (Boost REQUIRED COMPONENTS system filesystem program_options)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

set(USED_LIBS ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} jsoncpp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

add_executable(${PROJECT_NAME} ${CPPS})
target_link_libraries(${PROJECT_NAME} ${USED_LIBS})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
