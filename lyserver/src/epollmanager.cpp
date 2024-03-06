#include "epollmanager.h"
namespace lyserver
{
    static lyserver::Logger::ptr g_logger = LY_LOG_NAME("system");
    Epollmanager::Epollmanager()
    {
        epfd = create_epoll();
        event = 0;
    }
    Epollmanager::~Epollmanager()
    {
    }
    // 添加事件
    bool Epollmanager::eventadd(int events, lyserver::My_events *myev, const int &epfd)
    {
        struct epoll_event ev = {0, {0}};
        // 传统epoll模型传入lfd或cfd，epoll反应堆传入自定义结构体指针
        ev.data.ptr = myev;
        ev.events = events; // EPOLLIN 或EPOLLOUT
        // ev.events = events;
        RWMutex::WriteLock lock(m_mutex);
        myev->m_events = events;

        int op = 0;
        if (myev->m_status == 0)
        { // 若原来不再树上
            myev->m_status = 1;
            op = EPOLL_CTL_ADD;
            int ret;
            ret = epoll_ctl(epfd, op, GET_SOCK_FD(myev), &ev);

            if (ret == 0)
            {

                // printf("event add succeed[fd=%d][events=%d]\n", myev->m_fd, myev->m_events);
                return true;
            }
            else if (ret < 0)
            {
                if (errno == EBADF)
                {
                    LY_LOG_ERROR(g_logger) << "无效文件描述符";
                }
                else if (errno == EEXIST)
                {
                    LY_LOG_ERROR(g_logger) << "文件描述符已经存在于该树";
                }
                else if (errno == EINVAL)
                {
                    LY_LOG_ERROR(g_logger) << "事件类型不合法，或者其他参数不合法";
                }
                else if (errno == ENOMEM)
                {
                    LY_LOG_ERROR(g_logger) << "内存不足，无法分配内存以执行操作";
                }
                else
                {
                    LY_LOG_ERROR(g_logger) << "错误" << strerror(errno);
                }
                LY_LOG_ERROR(g_logger) << " event add/mod false [fd=" << GET_SOCK_FD(myev) << "] [events= " << myev->m_events << "]";
                myev->clear(); // 然后清空客户端类的信息
                close(GET_SOCK_FD(myev));
                return false;
            }
        }
        else
        {
            LY_LOG_ERROR(g_logger) << "add error:already on tree";
        }
        return false;
    }
    // 删除一个事件
    bool Epollmanager::eventdel(lyserver::My_events *myev)
    {
        RWMutex::WriteLock lock(m_mutex);
        struct epoll_event epv;
        if (myev->m_status != 1)
        { // 树上没有这个文件操作符
            LY_LOG_ERROR(g_logger) << "该树上没有该文件描述符";
            return false;
        }
        myev->m_status = 0;
        epv.data.ptr = NULL;
        int ret;
        ret = epoll_ctl(myev->epfd, EPOLL_CTL_DEL, GET_SOCK_FD(myev), &epv);

        if (ret == 0)
        {
            return true;
        }
        else if (ret == -1)
        {
            if (errno == ENOENT)
            {
                LY_LOG_ERROR(g_logger) << "删除结点出现错误，不存在于该树上";
            }
            else
            {
                LY_LOG_ERROR(g_logger) << "错误发生在eventdel, " << strerror(errno);
            }
        }
        return false;
    }
    int Epollmanager::create_epoll()
    {
        // 创建epoll对象
        int epfd = epoll_create(MAX_EVENTS);

        if (epfd < 0)
        {
            LY_LOG_ERROR(g_logger) << "int create epfd failed :" << strerror(errno);
        }
        return epfd;
    }
    Epollmanager Epollmanager::create_epollmgr()
    {
        // 创建epoll对象
        int epfd = epoll_create(MAX_EVENTS);

        if (epfd < 0)
        {
            LY_LOG_ERROR(g_logger) << "int create epfd failed :" << strerror(errno);
        }
        return Epollmanager(epfd);
    }
    void Epollmanager::reset_oneshot(My_events *myev, int event_, const CALL_BACK &cb)
    {
        if (cb)
        {
            myev->Myev_set(cb);
        }

        epoll_event event;
        event.data.ptr = myev;
        event.events = event_ | EPOLLET | EPOLLONESHOT;
        myev->m_events = event_;
        int result;
        result = epoll_ctl(myev->epfd, EPOLL_CTL_MOD, GET_SOCK_FD(myev), &event);
        if (result == -1)
        {
            // epoll_ctl 出错，输出错误信息，包括行号
            LY_LOG_ERROR(g_logger) << "epoll_ctl error at " << myev->ID << ":" << strerror(errno) << myev->epfd;
        }
    }
    int Epollmanager::get_epfd()
    {
        return epfd;
    }

}