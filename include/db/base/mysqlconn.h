#ifndef MYSQLCONN_H
#define MYSQLCONN_H
#include <mysql/mysql.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <memory>
using namespace std;
using namespace chrono;
namespace lyserver
{
    class Mysqlconn
    {
    public:
        typedef std::shared_ptr<Mysqlconn> ptr;
        Mysqlconn();
        ~Mysqlconn();
        /*
         *@功能：连接mysql
         */
        bool connect(std::string user, std::string password, std::string dbname, std::string ip, unsigned short port = 3306);
        /*
         *@功能：更新数据库insert update delete
         */
        bool update(std::string sql);
        /*
         *@功能：查询数据库
         */
        int query(std::string sql);
        /*
         *@功能：获取结果集的下一行数据
         */
        bool next();
        /*
         *@功能：在结果集中，获取某一行的一列字段值
         */
        std::string value(int index);
        /*
         *@功能：获取结果集的列数
         */
        int get_column();
        /*
         *@功能：获取结果集的列名
         */
        std::vector<std::string> get_column_name();

        // int get_column_name();
        /*
         *@功能：事务操作
         */
        bool transaction();
        /*
         *@功能：提交事务
         */
        bool commit();
        /*
         *@功能：事务回滚
         */
        bool rollback();
        /*
         *@功能：刷新起始的空闲时间点
         */
        void refreshAliveTime();
        /*
         *@功能：计算连接存活的总时间长
         */
        long long getAliveTime();

    private:
        /*
         *@功能：释放结果集
         */
        void freeResult();

        MYSQL *m_conn = nullptr;
        MYSQL_RES *m_result = nullptr; // 结果集
        MYSQL_ROW m_row = nullptr;     // 一行的数据，指针数组
        MYSQL_FIELD *m_clo_name;       // 列名集合

        std::chrono::steady_clock::time_point m_alivetime; // 时钟
    };

}
#endif