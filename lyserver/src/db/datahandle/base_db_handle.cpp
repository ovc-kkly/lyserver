/*
 * @Author: ly ly1285620755@163.com
 * @Date: 2024-01-24 17:16:52
 * @LastEditors: ly ly1285620755@163.com
 * @LastEditTime: 2024-02-09 02:29:30
 * @FilePath: /lyserver_master/lyserver/src/db/datahandle/base_db_handle.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "base_db_handle.h"
namespace lyserver
{
    static lyserver::Logger::ptr g_logger = LY_LOG_NAME("system");
    std::shared_ptr<ConnectionPool> Db_handle::Conn_Pool; // 初始化连接池智能指针所管理的内存
    Db_handle::Db_handle(/* args */)
    {
        if (Conn_Pool.use_count() == 0)
        {
            Conn_Pool.reset(ConnectionPool::getConnectPool());
            LY_LOG_INFO(g_logger) << "连接池的数量:" << Conn_Pool->getconnectionNumber();
        }
    }

    Db_handle::~Db_handle()
    {
    }
    std::string Db_handle::insert_sql(const std::string &tableName, const std::map<std::string, std::string> &columns_values)
    {
        std::string columns = "";
        std::string values = "";
        for (auto &it : columns_values)
        {
            columns += it.first + ",";
            values += "'"+it.second + "',";
        }
        columns.pop_back();
        values.pop_back();
        std::string sql = "insert into " + tableName + "(" + columns + ") values(" + values + ")";
        return sql;
    }
    std::string Db_handle::update_sql(const std::string &tableName, const std::string &condition, const std::map<std::string, std::string> &columns_values)
    {
        std::string sql = "update " + tableName + " set ";
        for (auto it = columns_values.begin(); it != columns_values.end(); it++)
        {
            sql += it->first + " = '" + it->second + "',";
        }
        sql.pop_back(); // 去除最后一个逗号
        if(condition == ""){
            return sql;
        }
        sql += " where " + condition;
        return sql;
    }
    std::string Db_handle::delete_sql(const std::string &tableName, const std::string &condition)
    {
        if(condition == ""){
            std::string sql = "delete from " + tableName;
            return sql;
        }
        std::string sql = "delete from " + tableName + " where " + condition;
        return sql;
    }
    std::string Db_handle::create_sql(const std::string &tableName, const std::map<std::string, std::string> &columns_values)
    {
        std::string element = "";
        for (auto &it : columns_values)
        {
            element += it.first +" "+ it.second +" ,";
        }
        element.pop_back();
        std::string sql = "create table " + tableName + " (" + element + ")";
        return sql;
    }



    bool Db_handle::update_mysql(const std::string &sql)
    {
        shared_ptr<Mysqlconn> conn_ptr = Conn_Pool->getConnection();
        if (conn_ptr->update(sql) == true)
        {
            LY_LOG_INFO(g_logger) << "更新成功";
            return true;
        }
        else
        {
            LY_LOG_ERROR(g_logger) << "更新失败";
        }
        return false;
    }
    int Db_handle::query_mysql(const std::string &sql, unordered_map<string, string> &m)
    {
        shared_ptr<Mysqlconn> conn_ptr = Conn_Pool->getConnection();
        int ret = conn_ptr->query(sql);
        if (ret > 0)
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
            LY_LOG_ERROR(g_logger) << "没有该数据";
            return 0;
        }
        else if (ret == -1)
        {
            LY_LOG_ERROR(g_logger) << "查询失败";
            return -1;
        }
        return ret;
    }

}