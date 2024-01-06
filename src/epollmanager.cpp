#include "epollmanager.h"
namespace lyserver
{
    Epollmanager::Epollmanager()
    {
        epfd = create_epoll();
        epfd_subreactor = create_epoll();
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
            ret = epoll_ctl(epfd, op, myev->m_fd, &ev);
            // if (obj == main_re)
            // {
            //     ret = epoll_ctl(epfd, op, myev->m_fd, &ev);
            // }
            // else if (obj == sub_re)
            // {
            //     ret = epoll_ctl(epfd, op, myev->m_fd, &ev);
            // }

            if (ret == 0)
            {

                // printf("event add succeed[fd=%d][events=%d]\n", myev->m_fd, myev->m_events);
                return true;
            }
            else if (ret < 0)
            {
                if (errno == EBADF)
                {
                    printf("无效文件描述符");
                }
                else if (errno == EEXIST)
                {
                    printf("文件描述符已经存在于该树");
                }
                else if (errno == EINVAL)
                {
                    printf("事件类型不合法，或者其他参数不合法");
                }
                else if (errno == ENOMEM)
                {
                    printf("内存不足，无法分配内存以执行操作");
                }
                else
                {
                    printf("错误%s\n", strerror(errno));
                }
                printf("\n event add/mod false [fd= %d] [events= %d] \n", myev->m_fd, myev->m_events);
                myev->clear(); // 然后清空客户端类的信息
                // fdmagr->clear_ID_FD(myev); // 然后删除全局变量ID_FD中的键值对
                close(myev->m_fd);
                return false;
            }
        }
        else
        {
            printf("add error:already on tree\n");
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
            printf("该树上没有该文件描述符\n");
            return false;
        }
        myev->m_status = 0;
        epv.data.ptr = NULL;
        int ret;
        ret = epoll_ctl(*(myev->epfd), EPOLL_CTL_DEL, myev->m_fd, &epv);

        if (ret == 0)
        {
            return true;
        }
        else if (ret == -1)
        {
            if (errno == ENOENT)
            {
                printf("删除结点出现错误，不存在于该树上");
            }
            else
            {
                printf("错误发生在eventdel, %s", strerror(errno));
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
            printf("create epfd in %s failed: %s \n", __func__, strerror(errno));
        }
        return epfd;
    }
    void Epollmanager::reset_oneshot(My_events *myev, int event_, const CALL_BACK &cb)
    {
        myev->Myev_set(cb);
        epoll_event event;
        event.data.ptr = myev;
        event.events = event_ | EPOLLET | EPOLLONESHOT;
        myev->m_events = event_;
        int result;
        result = epoll_ctl(*(myev->epfd), EPOLL_CTL_MOD, myev->m_fd, &event);
        // if (obj == main_re)
        // {
        //     result = epoll_ctl(*(myev->epfd), EPOLL_CTL_MOD, myev->m_fd, &event);
        // }
        // else
        // {
        //     result = epoll_ctl(*(myev->epfd), EPOLL_CTL_MOD, myev->m_fd, &event);
        // }
        if (result == -1)
        {
            // epoll_ctl 出错，输出错误信息，包括行号
            std::cerr << "epoll_ctl error at line " << myev->ID << ":" << __LINE__ << ": " << strerror(errno) << std::endl;
        }
    }
    int Epollmanager::get_epfd(reactor_type obj)
    {
        if (obj == main_re)
        {
            return epfd;
        }
        else
        {
            return epfd_subreactor;
        }
    }

}