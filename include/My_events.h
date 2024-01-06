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

namespace lyserver
{
#define BUFLEN 4096 // 缓存区大小

    using CALL_BACK = std::function<void()>; // 包装器
    // 自定义任务类
    class My_events
    {
    public:
        typedef std::shared_ptr<My_events> ptr;
        My_events();
        ~My_events();
        void init();

        void clear();
        // 更改
        void Myev_set(CALL_BACK callback);
        // 重载更改
        void Myev_set(int fd, CALL_BACK callback, struct sockaddr_in &cliaddr,const int *epfd);
        void Myev_set(int fd, CALL_BACK callback,const int *epfd);

        template <typename F, typename... Args>
        static CALL_BACK make_callback(F &&f, Args &&...args)
        {
            return std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        }
        // template <typename F, typename D, typename... Args>
        // static CHECK_CB make_callback(F &&f, D &&d, Args &&...args)
        // {
        //     return std::bind(std::forward<F>(f), std::forward<D>(d), std::forward<Args>(args)...);
        // }

        inline int getfd() const { return m_fd; }
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

        struct sockaddr_in getCliAddr() const
        {
            return cliaddr;
        }

        int m_fd;            // 监听的文件描述符
        const int *epfd;           // 自己所在的epfd
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

        struct sockaddr_in cliaddr; // 保存对方的IP和端口
    };

}
#endif
