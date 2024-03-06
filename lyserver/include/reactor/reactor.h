#ifndef REACTOR_H
#define REACTOR_H
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
#include "sig_handle.h"
#include "macro.h"
#include <queue>

namespace lyserver
{
#define MAX_EVENTS 1024  // 监听上限
#define SERVER_PORT 8080 // 服务器端口号                                  //监听上限数
#define FTP_PORT 8999
#define FTP_DATA_PORT 8998

    // #define EPFD_ GET_EPFD(Epoll_reactor::epollmgr, reactor_type::main_re)
    // #define EPFD_SUB_ GET_EPFD(Epoll_reactor::epollmgr, reactor_type::sub_re)

#define EPFD(E) GET_EPFD(E->epollmgr)
    extern Signal g_received;
    class Reactor
    {
    public:
        Fdmanager::ptr fdmagr;
        EventHandle::ptr EventH;
        std::string server_Timer;
        typedef std::shared_ptr<Reactor> ptr;
        Reactor();

        virtual void registerHandler(My_events *myev, int events) = 0;
        virtual void removeHandler(My_events *myev) = 0;
        virtual void handleEvents(ThreadPool::ptr pool) = 0;
        virtual ~Reactor() {}
    };

    class EventHandler
    {
    public:
        virtual void handleEvent(My_events *myev, Fdmanager::ptr fdmagr, int listen_type, uint64_t m_recvTimeout) = 0;
        virtual ~EventHandler() {}
    };
    class Acceptor;
    class Connection;
    class MainReactor : virtual public Reactor, public TimerManager
    {
        friend class Acceptor;
        friend class Connection;

    public:
        typedef std::shared_ptr<MainReactor> ptr;
        MainReactor(const std::string& server_Timer);
        void init(ThreadPool::ptr p);
        void Eventloop(ThreadPool::ptr p);

        void detection_client(ThreadPool::ptr p, Fdmanager::ptr fdmagr);
        void client_notactive(My_events *myevent, Fdmanager::ptr fdmagr);
        void registerHandler(My_events *myev, int events) override;
        void removeHandler(My_events *myev) override;

        // void setcallback(const CALL_BACK &cb_) { cb = std::move(cb_); }
        void handleEvents(ThreadPool::ptr pool) override;

        bool stopping(uint64_t &timeout);
        /**
         * @brief 当有新的定时器插入到定时器的首部,执行该函数
         */
        virtual void onTimerInsertedAtFront();
        Epollmanager::ptr getEpollMgr() { return epollmgr; }

        EventHandle::ptr geteventh() { return EventH; }
        My_events filedata_event;

    protected:
        Epollmanager::ptr epollmgr;

    private:
        /// pipe 文件句柄
        int m_tickleFds[2];
        // 其他成员变量和方法
        My_events mg_events_Timer;

        std::queue<epoll_event> eventqueue;

        // 接受epoll_wait 返回
        struct epoll_event evs_main[MAX_EVENTS];
    };

    class SubReactor : virtual public Reactor
    {
        friend class Acceptor;
        friend class Connection;

    public:
        typedef std::shared_ptr<SubReactor> ptr;
        SubReactor();
        void subEventloop(ThreadPool::ptr p);
        void registerHandler(My_events *myev, int events) override;
        void removeHandler(My_events *myev) override;

        // void setcallback(const CALL_BACK &cb_) { cb = std::move(cb_); }
        void handleEvents(ThreadPool::ptr pool) override;
        Epollmanager::ptr getEpollMgr() { return epollmgr; }

    private:
        Epollmanager::ptr epollmgr;
        std::queue<epoll_event> eventqueue;
        // 其他成员变量和方法
        struct epoll_event evs_sub[MAX_EVENTS];
    };
    using call_back_ = std::function<int(Socket::ptr)>;
    class Connection : public EventHandler
    {
    public:
        typedef std::shared_ptr<Connection> ptr;
        Connection() {}
        Connection(SubReactor::ptr reactor);
        int JudgeEventType(const char *buff, My_events *myev);

        virtual void recvdata(My_events *myev, Fdmanager::ptr fdmagr);

        // 可写事件回调函数 发送数据
        void handleEvent(My_events *myev, Fdmanager::ptr fdmagr, int listen_type, uint64_t m_recvTimeout) override;

        void senddata(My_events *myev, Fdmanager::ptr fdmagr);
        void FTP_recvdata(My_events *myev, Fdmanager::ptr fdmagr);
        void handle_fileevent(My_events *myev, Fdmanager::ptr fdmagr);
        void handle_http_event(My_events *myev, Fdmanager::ptr fdmagr);

        void setcb(const CALL_BACK &cb_) { cb = std::move(cb_); }
        void setHttp_handle_cb(const call_back_ &cb_) { Http_handle_cb = std::move(cb_); }
    private:
        void update_time(My_events *myev);

    private:
        SubReactor::ptr reactor_;
        CALL_BACK cb;
        call_back_ Http_handle_cb;
    };
    class Acceptor : public EventHandler
    {
    public:
        typedef std::shared_ptr<Acceptor> ptr;
        bool stop();
        Acceptor() {}
        Acceptor(MainReactor::ptr reactorm, SubReactor::ptr reactors, Connection::ptr conn, std::string client_Timer);
        bool initlistensocket(Address::ptr addr, uint64_t m_recvTimeout);
        bool init_FTP_listen(Address::ptr addr, uint64_t m_recvTimeout);
        bool inti_file_listen(Address::ptr addr, uint64_t m_recvTimeout);
        bool init_http_listen(Address::ptr addr, uint64_t m_recvTimeout);
        bool init_UDP_listen(Address::ptr addr, uint64_t m_recvTimeout);

        void acceptconnect(My_events *myev, Fdmanager::ptr fdmagr, int listen_type, uint64_t m_recvTimeout);
        void filedata_acceptconnect(My_events *myev, Fdmanager::ptr fdmagr);
        void httpaccept(My_events *myev, Fdmanager::ptr fdmagr, uint64_t m_recvTimeout);
        void UDP_recv(My_events *myev, Fdmanager::ptr fdmagr, uint64_t m_recvTimeout);
        void handleEvent(My_events *myev, Fdmanager::ptr fdmagr, int listen_type, uint64_t m_recvTimeout) override;

        void start();
        bool start(serverType type, uint64_t m_recvTimeout);
        void addAddress(Address::ptr addr);
        void setcb(const CALL_BACK &cb_) { cb = std::move(cb_); }

    private:
        MainReactor::ptr reactor_m;
        SubReactor::ptr reactor_s;
        Connection::ptr conn_;
        Socket::ptr m_socket; // 监听socket

        Socket::ptr m_socket_FTP;
        Socket::ptr m_socket_FTPDATA;
        Socket::ptr m_socket_udp;
        Socket::ptr m_socket_http;
        Address::ptr addr;
        CALL_BACK cb;

        
        std::string client_Timer;
    };

}
#endif