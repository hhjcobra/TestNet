cmake_minimum_required(VERSION 3.6)
project(TestNet)

set(CMAKE_CXX_STANDARD 11)
set(CFLAGS "${CMAKE_CXX_FLAGS} -o1")
set(SOURCE_FILES main.cpp Epoll.cpp Select.cpp Net_Utility.cpp)
add_executable(TestNet ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(TestNet pthread)