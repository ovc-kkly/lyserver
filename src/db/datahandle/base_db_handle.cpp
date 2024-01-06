#include "base_db_handle.h"
namespace lyserver
{
    std::shared_ptr<ConnectionPool> Db_handle::Conn_Pool; // 初始化连接池智能指针所管理的内存
    Db_handle::Db_handle(/* args */)
    {
        if (Conn_Pool.use_count() == 0)
        {
            Conn_Pool.reset(ConnectionPool::getConnectPool());
            cout << Conn_Pool->getconnectionNumber() << endl;
        }
    }

    Db_handle::~Db_handle()
    {
    }

    bool Db_handle::update_mysql(const std::string &sql)
    {
        shared_ptr<Mysqlconn> conn_ptr = Conn_Pool->getConnection();
        if (conn_ptr->update(sql) == true)
        {
            cout << "更新成功" << endl;
            return true;
        }
        else
        {
            cout << "更新失败" << endl;
        }
        return false;
    }
    int Db_handle::query_mysql(const std::string &sql, unordered_map<string, string> &m)
    {
        shared_ptr<Mysqlconn> conn_ptr = Conn_Pool->getConnection();
        int ret = conn_ptr->query(sql);
        if (ret == 1)
        {
            while (conn_ptr->next())
            {
                vector<string> column_name = conn_ptr->get_column_name();

                for (int i = 0; i < conn_ptr->get_column(); i++)
                {
                    // cout << conn_ptr->value(i) << " " << endl;
                    m[column_name[i]] = conn_ptr->value(i);
                }
            }
        }
        else if (ret == 0)
        {
            cout << "没有该数据" << endl;
            return 0;
        }
        else if (ret == -1)
        {
            cout << "查询失败" << endl;
            return -1;
        }
        return 1;
    }
}