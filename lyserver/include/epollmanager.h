#ifndef EPOLLMANAGER_H
#define EPOLLMANAGER_H

#include <memory>
#include <sys/epoll.h>
#include "My_events.h"
#include "singleton.h"
#include "EventType.h"
#include "mutex.h"
#include "myutil.h"
#include "noncopyable.h"
// #include "fdmanager.h"
namespace lyserver
{
    // class My_events;

#define MAX_EVENTS 1024 // 监听上限F
#define GET_EPFD(x) x->get_epfd()
    using CALL_BACK = std::function<void()>; // 包装器
    class Epollmanager
    {
    public:
        typedef std::shared_ptr<Epollmanager> ptr;
        Epollmanager();
        Epollmanager(int epfd){this->epfd = epfd;}
        Epollmanager(const int* epoll){epfd = *epoll;}
        Epollmanager(const Epollmanager& obj)
        {
            this->epfd = obj.epfd;
            this->event = obj.event;
        }
        ~Epollmanager();
        /**
         * @brief 添加epfd中的事件
         * 
         * @param events 
         * @param myev 
         * @param epfd 
         * @return true 
         * @return false 
         */
        bool eventadd(int events, My_events *myev, const int &epfd);
        /**
         * @brief 删除事件
         * 
         * @param myev 
         * @return true 
         * @return false 
         */
        bool eventdel(My_events *myev);
        int create_epoll();
        Epollmanager create_epollmgr();
        /**
         * @brief epoll中的EPOLLONESHOT事件，每次回调之后需要重新修改事件
         *
         * @param myev 客户端的事件对象指针
         * @param obj 需要注册在哪个epoll对象上
         * @param event_ 注册的事件
         */
        void reset_oneshot(My_events *myev, int event_, const CALL_BACK &cb);
        int get_epfd();
        int get_event(){return event;}
        void event_plus(){this->event++;}
        void event_sub(){this->event--;}
    private:
        RWMutex m_mutex;
        int epfd;
        int event;
    };
}

#endif