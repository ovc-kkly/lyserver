/*
 * @Author: ly ly1285620755@163.com
 * @Date: 2024-01-24 17:16:53
 * @LastEditors: ly ly1285620755@163.com
 * @LastEditTime: 2024-02-09 02:40:23
 * @FilePath: /lyserver_master/lyserver/include/db/datahandle/base_db_handle.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef BASE_DB_HANDLE_H
#define BASE_DB_HANDLE_H
#include <memory>
#include <string>
#include "connectionPool.h"
#include <unordered_map>
#include "log.h"
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
         *          ==-1：查询失败
         *          == 0：没有数据
         *          > 0：查询成功,返回的是样本数，行数
         */
        int query_mysql(const std::string &sql, std::unordered_map<std::string, std::string> &m);
        inline std::string pack_str(const std::string &str)
        {
            return "'" + str + "'";
        }

        // 递归模板函数，用于处理单个参数
        template <typename T>
        std::string to_string(T &&arg)
        {
            // 对于字符串类型，直接返回
            if constexpr (std::is_same_v<std::decay_t<T>, std::string>)
            {
                return std::forward<T>(arg);
            }
            // 对于const char*类型，创建std::string对象
            else if constexpr (std::is_same_v<std::decay_t<T>, const char *>)
            {
                return std::string(std::forward<T>(arg));
            }
            // 对于其他类型（如数字），使用std::to_string转换
            else
            {
                return std::to_string(std::forward<T>(arg));
            }
        }

        // 递归模板函数，用于处理参数列表
        template <typename First, typename... Rest>
        std::string to_string(First &&first, Rest &&...rest)
        {
            return to_string(std::forward<First>(first)) + "," + to_string(std::forward<Rest>(rest)...);
        }

        // 递归模板函数的基本情况，当没有更多参数时返回空字符串
        inline std::string to_string()
        {
            return "";
        }

        template <typename... Args>
        std::string select_sql(const std::string &tableName, const std::string &condition, Args &&...args)
        {
            int num = sizeof...(args);
            // 生成参数列表的字符串表示
            std::string args_str = to_string(std::forward<Args>(args)...);
            if (num == 1 && args_str == "*")
            {
                if (condition == "")
                {
                    return "select * from " + tableName;
                }
                else
                {
                    return "select * from " + tableName + " where " + condition;
                }
            }
            if (condition == "")
            {
                return "select " + args_str + " from " + tableName;
            }
            std::string sql = "select " + args_str + " from " + tableName + " where " + condition;
            return sql;
        }
        std::string create_sql(const std::string &tableName, const std::map<std::string, std::string> &columns_values);
        /**
         * @brief sql的插入语句
         *
         * @param tableName
         * @param columns_values
         * @return std::string
         */
        std::string insert_sql(const std::string &tableName, const std::map<std::string, std::string> &columns_values);
        /**
         * @brief sql的更新语句
         *
         * @param tableName
         * @param condition
         * @param columns_values
         * @return std::string
         */
        std::string update_sql(const std::string &tableName, const std::string &condition, const std::map<std::string, std::string> &columns_values);
        /**
         * @brief sql的删除语句
         *
         * @param tableName
         * @param condition
         * @return std::string
         */
        std::string delete_sql(const std::string &tableName, const std::string &condition);
    };

}
#endif