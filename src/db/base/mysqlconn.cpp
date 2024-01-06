#include "mysqlconn.h"

namespace lyserver
{

    Mysqlconn::Mysqlconn()
    {
        m_conn = mysql_init(nullptr);
        mysql_set_character_set(m_conn, "utf8");
    }

    Mysqlconn::~Mysqlconn()
    {
        if (m_conn != nullptr)
        {
            mysql_close(m_conn);
        }
        freeResult();
    }

    bool Mysqlconn::connect(string user, string password, string dbname, string ip, unsigned short port)
    {
        MYSQL *ptr = mysql_real_connect(m_conn, ip.c_str(), user.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0);
        return ptr != nullptr;
    }

    bool Mysqlconn::update(string sql)
    {
        if (mysql_query(m_conn, sql.c_str()))
        {
            return false;
        }
        return true;
    }

    int Mysqlconn::query(string sql)
    {
        freeResult();                              // 清除结果集
        if (mysql_query(m_conn, sql.c_str()) == 0) // 查询，sql语句执行成功，返回0,否则非0
        {
            m_result = mysql_store_result(m_conn); // 将结果集从 mysql(参数) 对象中取出
            if (m_result)
            {
                // 使用mysql_num_rows获取结果集中的行数
                int num_rows = mysql_num_rows(m_result);

                if (num_rows > 0)
                {
                    // 有数据
                    // 可以在这里处理结果集或者返回true，根据具体需求
                    // ...
                    return 1;
                }
            }

            // 没有数据
            return 0;
        }
        // 查询失败
        return -1;
    }

    bool Mysqlconn::next()
    {
        if (m_result != nullptr)
        {
            m_row = mysql_fetch_row(m_result); // 返回二级指针，也就是指针数组
            if (m_row != nullptr)
            {
                return true;
            }
        }
        return false;
    }

    string Mysqlconn::value(int index)
    {
        int rowCount = get_column(); // 列数
        if (index >= rowCount || index < 0)
        {
            return string();
        }
        char *val = m_row[index];                                    // 获取指针数组的第index个元素，是一个字符串
        unsigned long length = mysql_fetch_lengths(m_result)[index]; // 获取特定列的长度

        return string(val, length);
    }

    int Mysqlconn::get_column()
    {
        return mysql_num_fields(m_result);
    }
    std::vector<string> Mysqlconn::get_column_name()
    {
        int rowcount = get_column();
        std::vector<string> v;
        // 获取列信息
        m_clo_name = mysql_fetch_fields(m_result);
        for (int i = 0; i < rowcount; ++i)
        {
            v.push_back(m_clo_name[i].name);
        }
        return v;
    }
    bool Mysqlconn::transaction()
    {

        return mysql_autocommit(m_conn, false);
    }

    bool Mysqlconn::commit()
    {
        return mysql_commit(m_conn);
    }

    bool Mysqlconn::rollback()
    {
        return mysql_rollback(m_conn);
    }

    void Mysqlconn::refreshAliveTime()
    {
        m_alivetime = steady_clock::now();
    }

    long long Mysqlconn::getAliveTime()
    {
        nanoseconds res = steady_clock::now() - m_alivetime;
        milliseconds millsec = duration_cast<milliseconds>(res);

        return millsec.count();
    }

    void Mysqlconn::freeResult()
    {
        if (m_result)
        {
            mysql_free_result(m_result);
            m_result = nullptr;
        }
    }
}