# aux_source_directory(./ SRC_LIST)# 搜索源文件，并且保存为变量SRC_LIST

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_definitions("-Wall -g")
set(CMAKE_CXX_STANDARD 17)
include_directories(/usr/local/include)# 指定静态库库文件路径
include_directories(/usr/include)
include_directories(/usr/include/jsoncpp)
link_directories(/usr/local/lib64)
include_directories(/usr/include/x86_64-linux-gnu/)# curl.h
link_directories(/usr/lib/x86_64-linux-gnu)# libcurl
link_directories(/usr/local/lib)
link_directories(/usr/lib64/mysql)

set(LIB_SRC
    log.cpp
    config.cpp
    reactor/My_events.cpp
    daemon.cpp
    Buffer.cpp
    epollmanager.cpp
    timer.cpp
    EventHandle.cpp
    EventType.cpp
    fdmanager.cpp
    mutex.cpp
    tool/myutil.cpp
    tool/env.cpp
    tool/base64.cpp
    ocr_car.cpp
    r_s_Msg.cpp
    signal/sig_handle.cpp
    ${PROJECT_SOURCE_DIR}/other/Path_Planning/AStar.cpp
    ${PROJECT_SOURCE_DIR}/other/RF/RF.cpp
    ${PROJECT_SOURCE_DIR}/other/py/python.cpp
    ${PROJECT_SOURCE_DIR}/other/RF/personalization.cpp
    bytearray.cpp
    reactor/reactor.cpp
    network/address.cpp
    network/socket.cpp
    Tcpserver/Tcpserver.cpp
    fiber/fiber.cpp
    fiber/scheduler.cpp
    fiber/iomanager.cpp
    hook/hook.cpp
    FTP/ftp.cpp
    FTP/ftp_cmd.cpp
    fd_manager.cpp
    http/http.cpp
    http/http11_parser.rl.cc
    http/httpclient_parser.rl.cc
    http/http_parser.cpp
    http/http_session.cpp
    http/http_server.cpp
    http/servlet.cpp
    http/http_connection.cpp
    http/uri.rl.cc
    FileWR/fileWR.cpp
    ThreadPool/ThreadPool.cpp
    ThreadPool/thread.cpp
    db/base/connectionPool.cpp
    db/base/mysqlconn.cpp
    db/datahandle/base_db_handle.cpp
    db/datahandle/user_db_handle.cpp
    ${PROJECT_SOURCE_DIR}/lyserver/streams/socket_stream.cpp
    ${PROJECT_SOURCE_DIR}/lyserver/streams/stream.cpp
    ${PROJECT_SOURCE_DIR}/lyserver/streams/zlib_stream.cpp
)


add_definitions(-Wno-builtin-macro-redefined)
add_library(lyserver SHARED ${LIB_SRC})#生成自己的动态库
redefine_file_macro(lyserver)



# 添加 python 库链接
find_package(Python 3.9 COMPONENTS Development)
# find_package(jsoncpp REQUIRED)
# find_package(CURL REQUIRED)
# find_package(CryptoPP REQUIRED)
# find_package(yaml-cpp REQUIRED)

# include_directories(${jsoncpp_INCLUDE_DIRS})
# include_directories(${CURL_INCLUDE_DIRS})
# include_directories(${CryptoPP_INCLUDE_DIRS})
# include_directories(${YAML_CPP_INCLUDE_DIRS})
if(Python_FOUND)
    # 将 Python 的头文件路径添加到项目中
    include_directories(${Python_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Python not found. Please install Python or adjust Python_INCLUDE_DIRS and Python_LIBRARIES variables.")
endif()

# find_package(Boost REQUIRED COMPONENTS python3)
# include_directories(${Boost_INCLUDE_DIRS})
# find_package(jsoncpp REQUIRED)
# include_directories(${jsoncpp_INCLUDE_DIRS}/jsoncpp)


# set(LIBS lyserver ${jsoncpp_LIBRARIES} pthread mysqlclient ${CURL_LIBRARIES} ${CrytoPP_LIBRARIES} yaml-cpp ${Python_LIBRARIES})
set(LIBS lyserver jsoncpp pthread mysqlclient curl crypto yaml-cpp ${Python_LIBRARIES})
# set(LIBS lyserver jsoncpp pthread mysqlclient curl crypto yaml-cpp)
ly_add_executable(ly_server "${PROJECT_SOURCE_DIR}/example/echoserver.cpp" lyserver "${LIBS}")
ly_add_executable(http_server "${PROJECT_SOURCE_DIR}/example/httpserver.cpp" lyserver "${LIBS}")
ly_add_executable(server "${PROJECT_SOURCE_DIR}/lyserver/test/test_tcp_server.cpp" lyserver "${LIBS}")
