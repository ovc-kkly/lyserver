cmake_minimum_required(VERSION 3.0)
project(ly_server)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/db/base)
include_directories(${PROJECT_SOURCE_DIR}/include/db/datahandle)
include_directories(${PROJECT_SOURCE_DIR}/include/ThreadPool)
include_directories(${PROJECT_SOURCE_DIR}/lib/aip)

add_subdirectory(${PROJECT_SOURCE_DIR}/lyserver/src)# 指定源文件目录
