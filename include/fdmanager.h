#ifndef FDMANAGER_H
#define FDMANAGER_H

#include <memory>
#include <vector>
#include <string>
#include <set>
#include <unordered_map>
#include <map>
#include "singleton.h"
#include "mutex.h"
#include "My_events.h"
#include "myutil.h"
#include "epollmanager.h"
namespace lyserver
{
    // class lyserver::My_events;
    // class lyserver::Epollmanager;
#define GET_ID_FD(X, count) X::GetInstance()->get_ID_FD(count)
#define GET_ID_FD_1(X) X::GetInstance()->get_ID_FD(1)
#define EVENT_SIZE 1024

    class Fdmanager
    {
    public:
        // My_events t;
        typedef std::shared_ptr<Fdmanager> ptr;

        typedef std::shared_ptr<std::unordered_map<std::string, int>> umap;
        Fdmanager(int block = 1);
        ~Fdmanager();

        struct eventblock
        {
            // My_events myev[EVENT_SIZE]; // 数组
            std::vector<My_events> myev;
            std::unordered_map<std::string, int> ID_FD; // 客户端的ID和fd
            eventblock *next;
            void init()
            {
                myev.reserve(EVENT_SIZE);
                ID_FD.reserve(5);
                next = nullptr;
            }
        };
        struct fdstorage
        {
            // std::vector<int> epfd;
            std::map<int, int> epfd;
            std::map<int, int> epfd_sub;
            int block_cnt;

            eventblock *event_blk_list; // 链表头结点
            /**
             * @brief 先构造内存
             *
             * @param block 几个块
             */
            fdstorage(int block)
            {
                event_blk_list = (eventblock *)malloc(sizeof(eventblock) * block);
                // event_blk_list = new eventblock[block];
                block_cnt = block;
                if (block == 1)
                {
                    event_blk_list->init();
                }
                else
                {
                    for (int i = 0; i < block; i++)
                    {
                        if (i + 1 == block)
                        {
                            event_blk_list->myev.reserve(EVENT_SIZE);
                            event_blk_list->ID_FD.reserve(5);
                            event_blk_list[i].next = nullptr;
                        }
                        else
                        {
                            event_blk_list->myev.reserve(EVENT_SIZE);
                            event_blk_list->ID_FD.reserve(5);
                            event_blk_list[i].next = &event_blk_list[i + 1];
                        }
                    }
                }

                if (event_blk_list == nullptr)
                {
                    std::cout << "创建块错误" << std::endl;
                }
                // epfd.reserve(EVENT_SIZE);
            }
            /**
             * @brief 释放申请的内存
             *
             * @return true
             * @return false
             */
            void delete_fds()
            {
                delete[] event_blk_list;
                event_blk_list = nullptr;
            }
        };
        typedef std::shared_ptr<fdstorage> ptr_fdstorage;
        /**
         * @brief 重新指定大小
         *
         * @param r
         * @return int
         */
        int resize();
        /**
         * @brief 从存储结构中找到一个位置返回
         *
         * @param r
         * @param fd
         * @return My_events*
         */
        My_events *lookup(int fd);
        /**
         * @brief 获取存储中的块数
         *
         */
        int get_block() { return m_fd_storage->block_cnt; }
        pair<map<int, int>::iterator, bool> addepfd(int fd, const reactor_type &epfdType);
        /**
         * @brief 获取存储结构中的ID_FD
         *
         * @param block
         * @return std::unordered_map<std::string, My_events>
         */
        umap get_ID_FD(int block = 1);

        /**
         * @brief 根据toID找到对应的fd
         *
         * @param toID
         * @return int
         */
        int ID_2_fd(const std::string &toID);
        /**
         * @brief 是否存在该ID
         *
         * @param ID
         * @return true
         * @return false
         */
        bool is_exist(const std::string &ID);
        /**
         * @brief 清除ID_FD中指定的键值对
         *
         * @param myev
         */
        void clear_ID_FD(My_events *myev);

        /**
         * @brief 判断是客户端的ID
         *
         * @param myev
         * @param er_ptr
         * @param pthread_ID
         * @return int
         */
        int whoID(My_events *myev, std::string &ID);
        /**
         * @brief 返回epfd所对应的监听数量
         *
         * @param epfd
         * @return int
         */
        int get_epfd_size(int &epfd);
        /**
         * @brief 释放客户端的资源
         *
         * @param myev
         * @param str
         * @param b
         */
        void free_client_resources(My_events *myev, const char *str, bool &&b);

        /**
         * @brief 从map中找到对应类型的且合适的epfd
         *
         * @param epfd_type
         * @return const int* 保存在map中的一个epfd的地址
         */
        const int *find_epfd(const reactor_type &epfd_type);
        /**
         * @brief 添加epfd的监听事件个数
         *
         * @param epfd
         * @param epfd_type
         */
        void add_epfd_event(const int &epfd, const reactor_type &epfd_type);
        /**
         * @brief 减少epfd的监听事件个数
         * 
         * @param epfd 
         * @param epfd_type 
         */
        void subtract_epfd_event(const int &epfd, const reactor_type &epfd_type);

    private:
        ptr_fdstorage m_fd_storage;   // 存储结构
        std::set<std::string> userID; // 服务器只认识的ID在这个集合中
        RWMutex m_mutex;
        umap Id_Fd;

        Epollmanager::ptr epollmagr;

        /// 是否用户主动设置非阻塞
        bool m_Nonblock;
        /// 是否关闭
        bool m_isClosed;
    };

    /// 文件句柄单例

    struct delete_fd_storage
    {
    public:
        void operator()(Fdmanager::fdstorage *fds)
        {
            fds->delete_fds();
        }
    };
    typedef Singleton<Fdmanager> FdMgr;

}

#endif