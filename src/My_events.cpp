#include "My_events.h"

namespace lyserver
{
    My_events::My_events() : m_events(0), m_status(0), m_buf_len(0), ID("unknown")
    {

        readBuffer.reset(new Buffer);
    }
    My_events::~My_events()
    {
    }
    void My_events::init()
    {
        readBuffer.reset(new Buffer);
        epfd = 0;
        m_events = 0;
        m_status = 0;
        m_buf_len = 0;
        ID = "unknown";
    }
    void My_events::Myev_set(CALL_BACK callback)
    {
        call_back = callback;
    }
    void My_events::Myev_set(int fd, CALL_BACK callback, struct sockaddr_in &cliaddr,const int *epfd)
    {
        call_back = callback;
        m_fd = fd;
        // time(&last_active);
        last_active = GetCurrentMS();
        this->cliaddr = cliaddr;
        this->epfd = epfd;
    }
    void My_events::Myev_set(int fd, CALL_BACK callback,const int *epfd)
    {
        call_back = callback;
        m_fd = fd;
        // time(&last_active);
        last_active = GetCurrentMS();
        this->epfd = epfd;
    }
    /*重新清0被赋值的变量*/
    void My_events::clear()
    {
        call_back = nullptr;
        m_events = 0;
        // m_fd = 0;
        readBuffer->clear();
        m_buf_len = 0;
        last_active = 0;
        // T.reset();
        type = -1;
        printf("重置完毕\n");
    }
}
