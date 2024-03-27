#include "My_events.h"

namespace lyserver
{
    static lyserver::Logger::ptr g_logger = LY_LOG_NAME("system");
    My_events::My_events() : m_events(0), m_status(0), m_buf_len(0), ID("unknown")
    {

        readBuffer.reset(new Buffer);
    }
    My_events::~My_events()
    {
    }
    void My_events::init()
    {
        // RWMutex::WriteLock lock(m_mutex);
        readBuffer.reset(new Buffer);
        epfd = 0;
        m_events = 0;
        m_status = 0;
        m_buf_len = 0;
        ID = "unknown";
        m_file_event = nullptr;
    }
    void My_events::Myev_set(const CALL_BACK &callback)
    {
        // RWMutex::WriteLock lock(m_mutex);
        call_back = callback;
    }
    void My_events::Myev_set(Socket::ptr sockptr, const CALL_BACK &callback, Address::ptr clientIP, int epfd)
    {
        // RWMutex::WriteLock lock(m_mutex);
        call_back = callback;
        // m_fd = fd;
        this->sockptr = sockptr;
        // time(&last_active);
        last_active = GetCurrentMS();
        this->clientIP = clientIP;
        this->epfd = epfd;
    }
    int My_events::Myev_set(Socket::ptr sockptr, const CALL_BACK &callback, int epfd)
    {
        // RWMutex::WriteLock lock(m_mutex);
        try
        {
            if (!callback)
            {
                LY_LOG_ERROR(g_logger) << "回调函数未创建成功";
                return -1;
            }
            call_back = callback;
            this->sockptr = sockptr;
            // time(&last_active);
            last_active = GetCurrentMS();
            this->epfd = epfd;
            return 1;
        }
        catch (const std::exception &e)
        {
            LY_LOG_ERROR(g_logger) << e.what();
        }
        return 0;
    }
    /*重新清0被赋值的变量*/
    void My_events::clear()
    {
        // RWMutex::WriteLock lock(m_mutex);
        call_back = nullptr;
        m_events = 0;
        // m_fd = 0;
        sockptr.reset();
        readBuffer->clear();
        m_buf_len = 0;
        last_active = 0;
        m_status = 0;
        if (T.use_count() > 0)
        {
            this->T->cancel();
        }

        type = -1;
        // LY_LOG_INFO(g_logger) << "重置完毕";
    }
    void My_events::clearhttp()
    {
        call_back = nullptr;
        m_events = 0;
        // m_fd = 0;
        sockptr.reset();
        m_status = 0;
    }
}
