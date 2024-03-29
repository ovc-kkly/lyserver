/**
 * @file reactor_old.h未使用，是旧版代码
 * @author 罗杨 (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-01-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef REACTOR__H
#define REACTOR__H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include "r_s_Msg.h"
#include <unordered_map>
#include <set>
#include <pthread.h>
#include <functional>
#include <json/json.h>
#include "socket.h"
#include "myutil.h"
#include "EventType.h"
#include "timer.h"
#include "user_db_handle.h"
#include "fdmanager.h"
#include "EventHandle.h"
#include "epollmanager.h"
#include "macro.h"
using namespace Json;
using namespace std;

namespace lyserver
{

#define MAX_EVENTS 1024  // 监听上限
#define SERVER_PORT 8080 // 服务器端口号                                  //监听上限数
#define FTP_PORT 8999
#define FTP_DATA_PORT 8998
    // 反应堆类
    class Epoll_reactor;
    class Epoll_reactor : public TimerManager
    {
    public:
        typedef std::shared_ptr<Epoll_reactor> ptr;
        // 构造函数，启动服务器
        Epoll_reactor();
        ~Epoll_reactor();
        /**
         * @brief 初始化监听套接字
         *
         */
        void initlistensocket(const char *address, uint16_t port = SERVER_PORT);
        void init_FTP_listen(const char *address, uint16_t port = FTP_PORT);
        void inti_file_listen(const char *address, uint16_t port = FTP_DATA_PORT);
        static void filedata_acceptconnect(Epoll_reactor *er_ptr, My_events *myev, Fdmanager::ptr fdmagr);
        static void handle_fileevent(Epoll_reactor *er_ptr, My_events *myev, Fdmanager::ptr fdmagr);
        /**
         * @brief 子
         *
         * @param er_ptr
         * @param p
         * @param fdmagr
         */
        static void subreactor(Epoll_reactor *er_ptr, ThreadPool::ptr p, Fdmanager::ptr fdmagr);
        //  static void acceptconnect(void* erptr, void *arg);
        static void acceptconnect(Epoll_reactor *er_ptr, My_events *myev, Fdmanager::ptr fdmagr, ThreadPool::ptr p, int listen_type = 0);
        static void udp_recv(int udpSocket);
        /**
         * @brief FTP的读事件回调函数
         *
         * @param er_ptr
         * @param myev
         * @param fdmagr
         */
        static void FTP_recvdata(Epoll_reactor *er_ptr, My_events *myev, Fdmanager::ptr fdmagr);
        // 可读事件回调函数 接受客户端发送的数据
        static void recvdata(Epoll_reactor *er_ptr, My_events *myev, Fdmanager::ptr fdmagr);
        // 可写事件回调函数 发送数据
        static void senddata(Epoll_reactor *er_ptr, My_events *myev, Fdmanager::ptr fdmagr);

        // 服务器运行
        void server_run();
        void update_time(My_events *myev);
        // void change_event(Epoll_reactor *ep_r, My_events *myev, int events);

        string get_info(const string &buff, const string &need);
        int send_function(My_events *myev, Fdmanager::ptr fdmagr);
        bool sendjson(int fd, string &buff);
        bool send_str(int fd, string &buff);
        int JudgeEventType(const char *buff, My_events *myev, Epoll_reactor *er_ptr);
        // void clear_mg(const string &ID, Epoll_reactor *EP);
        Epollmanager::ptr get_Epollmgr() { return epollmgr; }

        // 接受epoll_wait 返回
        struct epoll_event evs_main[MAX_EVENTS];
        struct epoll_event evs_sub[MAX_EVENTS];

        bool stopping(uint64_t &timeout);
        /**
         * @brief 当有新的定时器插入到定时器的首部,执行该函数
         */
        virtual void onTimerInsertedAtFront();

        static void detection_client(ThreadPool::ptr p, Fdmanager::ptr fdmagr);
        static void client_notactive(My_events *myevent, Fdmanager::ptr fdmagr);

    private:
        // 红黑树文件描述符
        Epollmanager::ptr epollmgr;
        Fdmanager::ptr fdmagr;
        // My_events mg_events[MAX_EVENTS];
        My_events mg_events_Timer;
        // Timerfd::ptr MyTimer;
        UdpReceiver *udp_l;

        /// pipe 文件句柄
        int m_tickleFds[2];

        reactor_type reactor_obj;

        /**
         * @brief 任务对象
         *
         */
        EventHandle::ptr EventH;
        My_events filedata_event;

    private:
        ThreadPool::ptr pool;

        static pthread_mutex_t mutex_epfd; // 状态锁
    };

#define EPFD_ GET_EPFD(Epoll_reactor::epollmgr, reactor_type::main_re)
#define EPFD_SUB_ GET_EPFD(Epoll_reactor::epollmgr, reactor_type::sub_re)

#define EPFD(E) GET_EPFD(E->Epoll_reactor::epollmgr, reactor_type::main_re)
#define EPFD_SUB(E) GET_EPFD(E->Epoll_reactor::epollmgr, reactor_type::sub_re)

    class EventHandler
    {
    public:
        virtual void handleEvent(int fd, uint32_t events) = 0;
    };

}

#endif