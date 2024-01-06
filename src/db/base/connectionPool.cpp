#include "connectionPool.h"
#include <fstream>

namespace lyserver
{

    ConnectionPool::~ConnectionPool()
    {
        while (m_connectionQ.empty())
        {
            Mysqlconn *conn = m_connectionQ.front();
            m_connectionQ.pop();
            delete conn;
        }
    }
    ConnectionPool::ConnectionPool()
    {
        // 加载Json的配置文件
        if (!parseJsonFile())
        {

            return;
        }
        // 创建连接队列
        for (uint32_t i = 0; i < m_minsize; i++)
        {
            addConnection();
        }
        // 创建一个生产者线程来创建连接
        std::thread producer(&ConnectionPool::produceConnection, this);
        // 创建一个管理者线程来销毁连接
        std::thread recycler(&ConnectionPool::recycleConnection, this);
        producer.detach();
        recycler.detach();
    }

    ConnectionPool *ConnectionPool::getConnectPool()
    {
        // static ConnectionPool pool; // 定义一个局部静态变量
        ConnectionPool* pool= lyserver::ConnPool::GetInstance();
        return pool;
    }
    int ConnectionPool::getconnectionNumber()
    {
        return this->m_connectionQ.size();
    }
    std::shared_ptr<Mysqlconn> ConnectionPool::getConnection()
    {
        std::unique_lock<std::mutex> locker(m_mutexQ);
        while (m_connectionQ.empty())
        {
            if (std::cv_status::timeout == m_consumer_cond.wait_for(locker, std::chrono::milliseconds(m_timeout)))
            {
                if (m_connectionQ.size())
                {
                    // return nullptr;
                    continue;
                }
            }
        }
        std::shared_ptr<Mysqlconn> connptr(m_connectionQ.front(), [this](Mysqlconn *conn)
                                      {
        std::lock_guard<std::mutex> locker(m_mutexQ);
        conn->refreshAliveTime();
        this->m_connectionQ.push(conn); });
        m_connectionQ.pop();
        m_produce_cond.notify_all();
        return connptr;
    }

    bool ConnectionPool::parseJsonFile()
    {
        std::fstream ifs("../../mysql_configuration/configuration.json");
        Json::Reader rd;
        Json::Value root;
        rd.parse(ifs, root);
        if (root.isObject())
        {
            m_ip = root["ip"].asString();
            m_port = root["port"].asInt();
            m_user = root["userName"].asString();
            m_passwd = root["password"].asString();
            m_dbName = root["dbName"].asString();
            m_minsize = root["minSize"].asInt();
            m_maxsize = root["maxSize"].asInt();
            m_maxIdleTime = root["maxIdleTime"].asInt();
            m_timeout = root["timeout"].asInt();
            return true;
        }
        return false;
    }

    void ConnectionPool::produceConnection()
    {
        // 不断生产连接数
        while (true)
        {
            std::unique_lock<std::mutex> locker(m_mutexQ);
            while (m_connectionQ.size() >= m_minsize) // 满足条件代表不用继续生产了
            {
                m_produce_cond.wait(locker); // 阻塞在这里
            }
            addConnection();              // 添加连接数量
            m_consumer_cond.notify_all(); // 生产完一个连接，唤醒其他条件变量阻塞的地方
        }
    }

    void ConnectionPool::recycleConnection()
    {
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // 每隔1秒，检测任务队列的数量和最小数量
            std::lock_guard<std::mutex> locker(m_mutexQ);
            while (m_connectionQ.size() > m_minsize)
            {
                Mysqlconn *conn = m_connectionQ.front();
                if (conn->getAliveTime() >= m_maxIdleTime)
                {
                    m_connectionQ.pop();
                    delete conn;
                }
                else
                {
                    break;
                }
            }
        }
    }

    void ConnectionPool::addConnection()
    {
        Mysqlconn *conn = new Mysqlconn;
        
        conn->connect(m_user, m_passwd, m_dbName, m_ip, m_port);
        conn->refreshAliveTime();
        m_connectionQ.push(conn);
    }

}
