cmake_minimum_required(VERSION 3.5.1)
project(stun_monitor)

set(CMAKE_CXX_STANDARD 14)

find_package (Boost REQUIRED COMPONENTS system filesystem program_options)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

find_package(Threads)
include_directories(SYSTEM ${Threads_INCLUDE_DIR})

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

file(GLOB CPPS "source/*.cpp")
file(GLOB HS "source/headers/*.h")
#include_directories("./source/headers/")

add_executable(${PROJECT_NAME} ${CPPS} ${HS})

target_link_libraries(${PROJECT_NAME} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES})

set(LIB_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/lib/)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

install(FILES ./source/src_script/script.py DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(FILES ./source/src_script/data_analysis.py DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/src_script)
install(FILES ./source/src_script/parse_command_line.py DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/src_script)
install(FILES ./source/src_script/parse_config.py DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/src_script)
install(FILES ./source/src_script/send_message.py DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/src_script)

install(FILES ./etc/config.default.json DESTINATION ${CMAKE_INSTALL_PREFIX}/etc)
install(FILES ./etc/config_script.default.json DESTINATION ${CMAKE_INSTALL_PREFIX}/etc)