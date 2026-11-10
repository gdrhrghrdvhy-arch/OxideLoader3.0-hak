cmake_minimum_required(VERSION 3.22.1)
project(oxide_cheat)

add_library(oxide_cheat SHARED oxide_cheat.cpp)

find_package(Threads REQUIRED)

target_link_libraries(oxide_cheat 
    android 
    log 
    Threads::Threads
)
