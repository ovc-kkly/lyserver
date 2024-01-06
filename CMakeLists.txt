cmake_minimum_required(VERSION 3.0)
project(ly_server)
# set(CMAKE_C_COMPILER /usr/local/gcc-11.3/bin/gcc)
# set(CMAKE_CXX_COMPILER /usr/local/gcc-11.3/bin/g++)
include_directories(${PROJECT_SOURCE_DIR}/lyserver/include)
include_directories(${PROJECT_SOURCE_DIR}/lyserver/include/db/base)
include_directories(${PROJECT_SOURCE_DIR}/lyserver/include/db/datahandle)
# include_directories(${PROJECT_SOURCE_DIR}/include/Eventcallback)
# include_directories(${PROJECT_SOURCE_DIR}/include/datahandle)
include_directories(${PROJECT_SOURCE_DIR}/lyserver/include/ThreadPool)
include_directories(${PROJECT_SOURCE_DIR}/lyserver/lib/aip)
# include_directories(${PROJECT_SOURCE_DIR}/include/car_ocr)

add_subdirectory(${PROJECT_SOURCE_DIR}/lyserver/src)# 指定源文件目录和可执行文件的存放位置${PROJECT_SOURCE_DIR}/bin/server