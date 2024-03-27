#ifndef MY_EVENTS_H
#define MY_EVENTS_H

#include <time.h>
#include <cstring>
#include <string>
#include <utility>
#include <functional>
#include <netinet/in.h>
#include "Buffer.h"
#include "myutil.h"
#include "timer.h"
#include "EventType.h"
#include "log.h"
#include "socket.h"

namespace lyserver
{
#define BUFLEN 4096 // 缓存区大小

    using CALL_BACK = std::function<void()>; // 包装器
#define GET_SOCK_FD(x) (x->sockptr->getSocket())
    // 自定义任务类
    class My_events
    {
    public:
        typedef std::shared_ptr<My_events> ptr;
        My_events();
        ~My_events();
        void init();

        void clear();
        void clearhttp();
        // 更改
        void Myev_set(const CALL_BACK &callback);
        // 重载更改
        void Myev_set(Socket::ptr sockptr, const CALL_BACK &callback, Address::ptr clientIP, int epfd);
        int Myev_set(Socket::ptr sockptr, const CALL_BACK &callback, int epfd);

        template <typename F, typename O, typename... Args>
        static CALL_BACK make_callback(F &&f, O &&obj, Args &&...args)
        {
            return std::bind(std::forward<F>(f), std::forward<O>(obj), std::forward<Args>(args)...);
        }
        // template <typename F, typename D, typename... Args>
        // static CHECK_CB make_callback(F &&f, D &&d, Args &&...args)
        // {
        //     return std::bind(std::forward<F>(f), std::forward<D>(d), std::forward<Args>(args)...);
        // }

        // inline int getfd() const { return m_fd; }
        CALL_BACK get_call_back() const { return call_back; }
        int get_event() const { return m_events; }
        int get_status() const
        {
            return m_status;
        }
        Buffer::ptr get_buffer() const { return readBuffer; }
        int getBufLen() const
        {
            return m_buf_len;
        }

        std::string getID() const
        {
            return ID;
        }

        time_t getLastActive() const
        {
            return last_active;
        }

        // Timerfd::ptr getTimer() const
        // {
        //     return T;
        // }

        int getType() const
        {
            return type;
        }

        Address::ptr getCliAddr() const
        {
            return clientIP;
        }

        // RWMutex m_mutex;
        // int m_fd;            // 监听的文件描述符
        Socket::ptr sockptr;
        My_events *m_file_event;

        int epfd; // 自己所在的epfd
        reactor_type epfd_type;
        CALL_BACK call_back; // 回调函数

        int m_events; // 监听事件类型 EPOLLIN EPOLLOUT
        // string m_events_str;
        int m_status; // 是否在监听 1在树上，0 不在
        // char m_buf[BUFLEN];
        Buffer::ptr readBuffer;
        int m_buf_len;

        std::string ID;
        uint64_t last_active; // 每次加入红黑树时间
        // Timerfd::ptr T;
        Timer::ptr T;
        int type; // 0代表是监听文件描述符，1代表是定时器

        Address::ptr clientIP; // 保存对方的IP和端口
    };

}
#endif
