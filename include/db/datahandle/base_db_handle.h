#ifndef BASE_DB_HANDLE_H
#define BASE_DB_HANDLE_H
#include <memory>
#include <string>
#include "connectionPool.h"
#include <unordered_map>

namespace lyserver
{
    class Db_handle
    {
    private:
        // 数据库智能指针
        typedef std::shared_ptr<Db_handle> dbh_ptr;

    protected:
        static std::shared_ptr<ConnectionPool> Conn_Pool;

    public:
        Db_handle(/* args */);
        ~Db_handle();

        /**
         * @brief 更新数据库的数据，包括插入、删除、更新
         *
         * @param sql
         * @return true
         * @return false
         */
        bool update_mysql(const std::string &sql);
        /**
         * @brief 查询数据库的数据
         *
         * @param sql
         * @param m
         * @return int
         */
        int query_mysql(const std::string &sql, std::unordered_map<std::string, std::string> &m);
    };

}
#endif