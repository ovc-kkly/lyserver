#include "fdmanager.h"
#include <sys/epoll.h>
namespace lyserver
{

    Fdmanager::Fdmanager(int block)
    {
        userID.insert("Qt");
        userID.insert("battery");
        userID.insert("xavier");
        userID.insert("action");
        userID.insert("uportrait");
        userID.insert("imx6ull");
        m_fd_storage.reset(new fdstorage(block), delete_fd_storage());
        Id_Fd = this->get_ID_FD(1);
        epollmagr.reset(Epollmagr::GetInstance()); // epoll管理
    }
    Fdmanager::~Fdmanager()
    {
        m_fd_storage.reset();
    }

    int Fdmanager::resize()
    {
        if (m_fd_storage.use_count() == 0)
            return -1;

        struct eventblock *blk = m_fd_storage->event_blk_list;

        while (blk != NULL && blk->next != NULL)
        {
            blk = blk->next;
        }
        // 申请1024个fd内存
        // My_events *myev = new [MAX_EVENTS];
        std::vector<My_events> myev(EVENT_SIZE);
        if (myev.size() != EVENT_SIZE)
            return -4;

        // 每个块,1个快保存一个items
        struct eventblock *block = (struct eventblock *)malloc(sizeof(eventblock));
        if (block == NULL)
        {
            myev.clear();
            myev.shrink_to_fit();
            return -5;
        }
        // memset(block, 0, sizeof(eventblock));
        block->init();

        block->myev = myev;
        block->next = NULL;

        if (blk == NULL)
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
        if (m_fd_storage.use_count() == 0)
            return NULL;
        // if(r->evblk == NULL) return NULL;
        if (fd <= 0)
            return NULL;

        // printf("reactor_lookup --> %d\n", r->blkcnt);

        int blkidx = fd / EVENT_SIZE;
        while (blkidx >= m_fd_storage->block_cnt) // 因为sockfd不可能突增，所以该循环只执行一次，也可使用if
        {
            resize(); // 添加block
        }
        int i = 0;
        struct eventblock *blk = m_fd_storage->event_blk_list; // 拿到链表头结点
        while (i++ < blkidx && blk != NULL)
        {
            blk = blk->next;
        }

        return &blk->myev[fd % EVENT_SIZE];
    }
    pair<map<int, int>::iterator, bool> Fdmanager::addepfd(int fd, const reactor_type &epfdType)
    {
        pair<map<int, int>::iterator, bool> it;
        switch (epfdType)
        {
        case main_re:
            it = this->m_fd_storage->epfd.insert(std::pair<int, int>(fd, 0));
            return it;
        case sub_re:
            it = this->m_fd_storage->epfd_sub.insert(std::pair<int, int>(fd, 0));
            return it;
        default:
            break;
        }
        return pair<map<int, int>::iterator, bool>();
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
        RWMutex::WriteLock lock(m_mutex);
        auto it = Id_Fd->find(myev->ID);
        if (it != Id_Fd->end())
        {
            Id_Fd->erase(it);
            myev->ID = "unknown";
        }
        else
        {
            printf("ID_FD中不存在该键值对\n");
        }

        // pthread_mutex_unlock(&mutex_fd);
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
                Id_Fd->insert(std::pair<std::string, int>(ID, myev->m_fd));
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
        return this->m_fd_storage->epfd.at(epfd);
    }
    const int *Fdmanager::find_epfd(const reactor_type &epfd_type)
    {
        if (epfd_type == main_re)
        {
            auto it = this->m_fd_storage->epfd.begin();
            for (; it != this->m_fd_storage->epfd.end(); it++)
            {
                if (it->second < EVENT_SIZE)
                {
                    return &(it->first);
                }
            }
            int epfd = epollmagr->create_epoll();
            pair<map<int, int>::iterator, bool> it_ = this->addepfd(epfd, main_re);

            return &(it_.first->first);
        }
        else
        {
            auto it = this->m_fd_storage->epfd_sub.begin();
            for (; it != this->m_fd_storage->epfd_sub.end(); it++)
            {
                if (it->second < EVENT_SIZE)
                {
                    return &(it->first);
                }
            }
            int epfd_sub = epollmagr->create_epoll();
            pair<map<int, int>::iterator, bool> it_ = this->addepfd(epfd_sub, sub_re);
            return &(it_.first->first);
        }
    }
    // 释放客户端的资源
    void Fdmanager::free_client_resources(My_events *myev, const char *str, bool &&b)
    {
        epollmagr->eventdel(myev); // 先从红黑树上删除结点
        this->subtract_epfd_event(*(myev->epfd), myev->epfd_type);
        printf("\n [Client:%d] %s ,ID:%s\n", myev->m_fd, str, myev->ID.c_str());
        myev->clear(); // 然后清空客户端类的信息
        if (b == true)
        {
            clear_ID_FD(myev); // 然后删除全局变量ID_FD中的键值对
        }

        close(myev->m_fd);
        myev->m_fd = 0;
    }
    void Fdmanager::add_epfd_event(const int &epfd, const reactor_type &epfd_type)
    {
        if (epfd_type == reactor_type::main_re)
        {
            this->m_fd_storage->epfd[epfd]++;
        }
        else
        {
            this->m_fd_storage->epfd_sub[epfd]++;
        }
    }
    void Fdmanager::subtract_epfd_event(const int &epfd, const reactor_type &epfd_type)
    {
        if (epfd_type == reactor_type::main_re)
        {
            this->m_fd_storage->epfd[epfd]--;
        }
        else
        {
            this->m_fd_storage->epfd_sub[epfd]--;
        }
    }
}