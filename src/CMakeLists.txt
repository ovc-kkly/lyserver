# aux_source_directory(./ SRC_LIST)# 搜索源文件，并且保存为变量SRC_LIST
# aux_source_directory(./db/base DB_LIST)
# # aux_source_directory(./Eventcallback ECB_LIST)
# aux_source_directory(./db/datahandle DH_LIST)
set(LIB_SRC
    My_events.cpp
    Buffer.cpp
    epollmanager.cpp
    Udp.cpp
    timer.cpp
    EventHandle.cpp
    EventType.cpp
    fdmanager.cpp
    mutex.cpp
    myutil.cpp
    ocr_car.cpp
    r_s_Msg.cpp
    reactor.cpp
    ThreadPool/ThreadPool.cpp
    db/base/connectionPool.cpp
    db/base/mysqlconn.cpp
    db/datahandle/base_db_handle.cpp
    db/datahandle/user_db_handle.cpp
    main.cpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_definitions("-Wall -g")
set(CMAKE_CXX_STANDARD 17)

include_directories(/usr/local/include)# 指定静态库库文件路径
include_directories(/usr/include)
# include_directories(/usr/include/x86_64-linux-gnu/)# curl.h
# link_directories(/usr/lib/x86_64-linux-gnu)# libcurl
link_directories(/usr/local/lib)
link_directories(/usr/lib64/mysql)
add_executable(server ${LIB_SRC})


target_link_libraries(server jsoncpp pthread mysqlclient curl crypto)
