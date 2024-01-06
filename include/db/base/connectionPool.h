#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H
#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "mysqlconn.h"
#include <json/json.h>
#include "singleton.h"
namespace lyserver
{
    class ConnectionPool
    {
    public:
        ~ConnectionPool();
        ConnectionPool();
        ConnectionPool(const ConnectionPool &obj) = delete;
        ConnectionPool &operator=(const ConnectionPool &obj) = delete;
        /*
         *@获取连接池
         */
        static ConnectionPool *getConnectPool();
        /*
         *@获取一个连接
         */
        std::shared_ptr<Mysqlconn> getConnection();
        int getconnectionNumber();

    private:
        
        /*
         *@解析Json
         */
        bool parseJsonFile();
        /*
         *@生产连接数，在一个线程里运行
         */
        void produceConnection();
        /*
         *@管理者，在一个线程里运行
         */
        void recycleConnection();
        /*
         *@添加连接
         */
        void addConnection();

        std::string m_ip;
        std::string m_user;
        std::string m_passwd;
        std::string m_dbName;
        unsigned short m_port;
        uint32_t m_minsize;
        uint32_t m_maxsize;
        int m_timeout;
        int m_maxIdleTime;
        std::queue<Mysqlconn *> m_connectionQ; // 数据库连接队列
        std::mutex m_mutexQ;
        std::condition_variable m_consumer_cond;
        std::condition_variable m_produce_cond;
    };
    /// 日志器管理类单例模式
    typedef lyserver::Singleton<ConnectionPool> ConnPool;
}
#endif