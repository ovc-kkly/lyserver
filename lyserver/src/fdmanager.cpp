#include "fdmanager.h"
#include <sys/epoll.h>
namespace lyserver
{
    Fdmanager *Fdmanager::fdmgr = nullptr;
    static lyserver::Logger::ptr g_logger = LY_LOG_NAME("system");
    Fdmanager::Fdmanager(int block)
    {
        userID.insert("Qt");
        userID.insert("battery");
        userID.insert("xavier");
        userID.insert("action");
        userID.insert("uportrait");
        userID.insert("imx6ull");
        
        // 停车场客户端
        userID.insert("detection");
        userID.insert("clientpyw");

        userID.insert("edge_computing");
        m_fd_storage.reset(new fdstorage(block), delete_fd_storage());
        Id_Fd = this->get_ID_FD(1);
        // epollmagr.reset(Epollmagr::GetInstance()); // epoll管理
    }
    Fdmanager::~Fdmanager()
    {
        LY_LOG_ERROR(g_logger) << "释放内存";
        m_fd_storage.reset();
        LY_LOG_ERROR(g_logger) << "释放内存--";
    }

    void Fdmanager::setTimeout(int type, uint64_t v)
    {
        if (type == SO_RCVTIMEO)
        {
            m_recvTimeout = v;
        }
        else
        {
            m_sendTimeout = v;
        }
    }

    uint64_t Fdmanager::getTimeout(int type)
    {
        if (type == SO_RCVTIMEO)
        {
            return m_recvTimeout;
        }
        else
        {
            return m_sendTimeout;
        }
        return 0;
    }
    int Fdmanager::resize()
    {
        RWMutex::WriteLock lock(this->eventblock_mutex);
        if (m_fd_storage.use_count() == 0)
            return -1;

        eventblock *blk = m_fd_storage->event_blk_list;

        while (blk != nullptr && blk->next != nullptr)
        {
            blk = blk->next;
        }

        // 申请1024个fd内存
        std::vector<My_events> myev(EVENT_SIZE);
        if (myev.size() != EVENT_SIZE)
            return -4;

        // 每个块,1个块保存一个items
        eventblock *block = new (std::nothrow) eventblock;
        if (block == nullptr)
        {
            myev.clear();
            return -5;
        }

        block->init();
        block->myev = std::move(myev);
        block->next = nullptr;

        if (blk == nullptr)
        {
            m_fd_storage->event_blk_list = block;
        }
        else
        {
            blk->next = block;
        }

        ++(m_fd_storage->block_cnt);

        return 0;
    }
    My_events *Fdmanager::lookup(int fd)
    {
        if (m_fd_storage.use_count() == 0 || fd <= 0)
            return nullptr;

        int blkidx = fd / EVENT_SIZE;

        while (blkidx >= m_fd_storage->block_cnt)
        {
            resize(); // 如果需要的话进行调整大小
        }

        struct eventblock *blk = m_fd_storage->event_blk_list;

        for (int i = 0; i < blkidx && blk != nullptr; ++i)
        {
            blk = blk->next;
        }
        My_events *m = (blk != nullptr) ? &blk->myev[fd % EVENT_SIZE] : nullptr;
        if (m == &blk->myev[0])
        {
            LY_LOG_INFO(g_logger) << "第一个位置";
        }
        return m;
    }
    pair<map<int, Epollmanager>::iterator, bool> Fdmanager::addepfd(int fd, const reactor_type &epfdType, const Epollmanager &epmgr)
    {

        RWMutex::WriteLock lock(this->epfd_mutex);
        pair<map<int, Epollmanager>::iterator, bool> it;
        switch (epfdType)
        {
        case main_re:
            // LY_LOG_INFO(g_logger) << "main_re" << fd;
            it = this->m_fd_storage->epfd.insert(std::pair<int, Epollmanager>(fd, epmgr));
            return it;
        case sub_re:
            // LY_LOG_INFO(g_logger) << "sub_re" << fd;
            it = this->m_fd_storage->epfd_sub.insert(std::pair<int, Epollmanager>(fd, epmgr));
            return it;
        default:
            break;
        }
        return pair<map<int, Epollmanager>::iterator, bool>();
    }
    Fdmanager::umap Fdmanager::get_ID_FD(int block)
    {
        RWMutex::ReadLock lock(m_mutex);
        eventblock *eb = this->m_fd_storage->event_blk_list;
        for (int i = 1; i < block; i++)
        {
            eb = eb->next;
        }

        return umap(&(eb->ID_FD));
    }

    void Fdmanager::clear_ID_FD(My_events *myev)
    {
        // pthread_mutex_lock(&mutex_fd);
        RWMutex::WriteLock lock(this->m_mutex);
        auto it = Id_Fd->find(myev->ID);
        if (it != Id_Fd->end())
        {
            Id_Fd->erase(it);
            myev->ID = "unknown";
        }
        else
        {
            LY_LOG_ERROR(g_logger) << "ID_FD中不存在该键值对";
        }
    }

    bool Fdmanager::is_exist(const string &ID)
    {
        RWMutex::ReadLock lock(m_mutex);
        if (Id_Fd->find(ID) != Id_Fd->end())
        {

            return true;
        }
        return false;
    }
    int Fdmanager::ID_2_fd(const string &toID)
    {
        int fd;
        RWMutex::ReadLock lock(m_mutex);
        if (Id_Fd->find(toID) != Id_Fd->end())
        {
            fd = Id_Fd->at(toID);
        }
        else
        {
            return -1;
        }
        return fd;
    }
    int Fdmanager::whoID(My_events *myev, string &ID)
    {
        if (this->is_exist(ID))
        { // 进入这里代表键值对中已经存在该ID
            return -2;
        }
        else
        { // 进入这里代表客户端还未登录
            RWMutex::WriteLock lock(m_mutex);
            if (userID.find(ID) != userID.end())
            { // 表示这个客户端的ID是服务器认识的
                myev->ID = ID;
                Id_Fd->insert(std::pair<std::string, int>(ID, GET_SOCK_FD(myev)));
                return 1;
            }
            else
            {
                return 0;
            }
        }
    }
    int Fdmanager::get_epfd_size(int &epfd)
    {
        return this->m_fd_storage->epfd.at(epfd).get_event();
    }
    const int *Fdmanager::find_epfd(const reactor_type &epfd_type, Epollmanager::ptr epollmagr)
    {
        if (epfd_type == main_re)
        {
            auto it = this->m_fd_storage->epfd.begin();
            for (; it != this->m_fd_storage->epfd.end(); it++)
            {
                if (it->second.get_event() < EVENT_SIZE)
                {
                    return &(it->first);
                }
            }
            LY_LOG_INFO(g_logger) << "no epfd";
            Epollmanager epmgr = epollmagr->create_epollmgr();
            pair<map<int, Epollmanager>::iterator, bool> it_ = this->addepfd(epmgr.get_epfd(), main_re, epmgr);

            return &(it_.first->first);
        }
        else
        {
            auto it = this->m_fd_storage->epfd_sub.begin();
            for (; it != this->m_fd_storage->epfd_sub.end(); it++)
            {
                if (it->second.get_event() < EVENT_SIZE)
                {
                    return &(it->first);
                }
            }
            LY_LOG_INFO(g_logger) << "no epfd";
            Epollmanager epmgr = epollmagr->create_epollmgr();
            pair<map<int, Epollmanager>::iterator, bool> it_ = this->addepfd(epmgr.get_epfd(), sub_re, epmgr);
            return &(it_.first->first);
        }
    }
    // 释放客户端的资源
    void Fdmanager::free_client_resources(My_events *myev, const char *str, bool &&b, Epollmanager *epollmagr)
    {
        epollmagr->eventdel(myev); // 先从红黑树上删除结点
        int event = this->subtract_epfd_event(myev->epfd, myev->epfd_type);
        LY_LOG_INFO(g_logger) << " [Client:" << GET_SOCK_FD(myev) << "] " << str << ",ID:" << myev->ID.c_str() << " epfd:" << myev->epfd << " event:" << event;
        myev->clear(); // 然后清空客户端类的信息
        if (b == true)
        {
            clear_ID_FD(myev); // 然后删除全局变量ID_FD中的键值对
        }
    }
    int Fdmanager::add_epfd_event(const int &epfd, const reactor_type &epfd_type)
    {
        RWMutex::WriteLock lock(this->epfd_mutex);
        if (epfd_type == reactor_type::main_re)
        {
            this->m_fd_storage->epfd[epfd].event_plus();
            return m_fd_storage->epfd[epfd].get_event();
        }
        else
        {
            this->m_fd_storage->epfd_sub[epfd].event_plus();
            return m_fd_storage->epfd_sub[epfd].get_event();
        }
        return -1;
    }
    int Fdmanager::subtract_epfd_event(const int &epfd, const reactor_type &epfd_type)
    {
        RWMutex::WriteLock lock(this->epfd_mutex);
        if (epfd_type == reactor_type::main_re)
        {
            this->m_fd_storage->epfd[epfd].event_sub();
            return m_fd_storage->epfd[epfd].get_event();
        }
        else
        {
            this->m_fd_storage->epfd_sub[epfd].event_sub();
            return m_fd_storage->epfd_sub[epfd].get_event();
        }
        return -1;
    }
    Epollmanager *Fdmanager::get_epollmgr(My_events *myev)
    {
        RWMutex::ReadLock lock(this->epfd_mutex);
        switch (myev->epfd_type)
        {
        case reactor_type::main_re:
            return &(this->m_fd_storage->epfd[myev->epfd]);

        case reactor_type::sub_re:
            return &(this->m_fd_storage->epfd_sub[myev->epfd]);
        default:
            break;
        }
        return nullptr;
    }
}