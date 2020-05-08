cmake_minimum_required(VERSION 2.8.10)
project(demo C CXX)

include_directories(${PROJECT_SOURCE_DIR}/
                    ${PROJECT_SOURCE_DIR}/include
                   )
link_directories(${PROJECT_SOURCE_DIR}/
                 ${PROJECT_SOURCE_DIR}/lib
                 ${PROJECT_SOURCE_DIR}/lib/boost_1_56_0
                )

link_libraries(libboost_random.a libboost_thread.a)
set(CMAKE_CXX_FLAGS "${CMAKE_CPP_FLAGS} -g -DNDEBUG -O0 -D__const__= -pipe -W -Wall -Wno-unused-parameter -fPIC -fno-omit-frame-pointer -std=c++11 -lpthread")
add_executable(demo demo.cpp)
target_link_libraries(demo libbaker-asr-linux.so libz.so libssl.so libcrypto.so)


