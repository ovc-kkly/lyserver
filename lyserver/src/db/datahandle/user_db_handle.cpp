/*
 * @Author: ly ly1285620755@163.com
 * @Date: 2024-01-24 17:16:52
 * @LastEditors: ly ly1285620755@163.com
 * @LastEditTime: 2024-02-09 02:20:43
 * @FilePath: /lyserver_master/lyserver/src/db/datahandle/user_db_handle.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "user_db_handle.h"

namespace lyserver
{
    static lyserver::Logger::ptr g_logger = LY_LOG_NAME("system");
    user_db_handle::user_db_handle(/* args */)
    {
        // std::cout << "引用计数:"<<Conn_Pool.use_count() << "地址:"<<&Conn_Pool<< "连接数量"<< Conn_Pool->getconnectionNumber()<<'\n';
    }

    user_db_handle::~user_db_handle()
    {
    }

    int user_db_handle::isExistUser(const string &userID)
    {
        // char str[50];
        // sprintf(str, "select * from User where name='%s'", userID.c_str());
        std::string sql = this->select_sql("User", "name=" + pack_str(userID), "*");
        int query_result = this->query_mysql(sql, m); // 把查询到的结果记录在类的成员变量m里
        return query_result;
    }
    bool user_db_handle::passward_right_wrong(const string &password)
    {
        if (m["password"] == password)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    bool user_db_handle::insert_userID(const string &userID, const string &password)
    {
        // char str[50];
        // sprintf(str, "insert into User(name,password) values('%s', '%s')", userID.c_str(), password.c_str());
        std::string sql = this->insert_sql("User", std::map<std::string, std::string>{
                                                       {"name", userID},
                                                       {"password", password}});
        // printf("%s\n", str);
        bool insert_result = this->update_mysql(sql);
        return insert_result;
    }
    int user_db_handle::query_user_info(const string &userID, const string &password)
    {
        // char str[50];
        // sprintf(str, "select name, password from User where name='%s'", userID.c_str());
        std::string sql = this->select_sql("User", "name=" +  pack_str(userID), "name", "password");
        unordered_map<string, string> m;
        int query_result = this->query_mysql(sql, m); // 把查询到的结果记录变量m里
        if (query_result > 0)
        {
            if (password == m["password"])
            {
                // sprintf(str, "delete from User where name='%s'", userID.c_str());
                std::string str = this->delete_sql("User", "name=" + pack_str(userID));
                return this->update_mysql(str) ? 1 : 0;
            }
            else
            { // 密码错误
                return -2;
            }
        }
        else
        {
            LY_LOG_INFO(g_logger) << "查询错误或用户不存在";
            return -1;
        }
    }
    void user_db_handle::change_user_log_on_state(const string &User_name, bool on_off)
    {
        if (on_off)
        {
            User_ID[User_name] = true;
        }
        else
        {
            User_ID.erase(User_name);
        }
    }
    void user_db_handle::delete_var()
    {
        if (!m.empty())
        {
            m.clear();
            LY_LOG_INFO(g_logger) << "释放结果的保存";
        }
    }
    bool user_db_handle::update_modeldir(const std::string &userID, const std::string &modeldir)
    {
        // char str[50];
        // sprintf(str, "update User set dir = %s where name = %s", modeldir.c_str(), userID.c_str());
        std::string sql = this->update_sql("User", "name=" +  pack_str(userID), std::map<std::string, std::string>{{"dir", modeldir}});
        bool update_result = this->update_mysql(sql);
        return update_result;
    }
    bool user_db_handle::select_modeldir(const std::string &userID, std::string &modeldir)
    {
        std::string sql = this->select_sql("User", "name=" +  pack_str(userID), "name", "dir");
        unordered_map<string, string> res;
        int query_result = this->query_mysql(sql, res);
        if (query_result > 0)
        { // 查询成功
            modeldir = res["dir"];
            return true;
        }
        else if (query_result == 0)
        {
            LY_LOG_ERROR(g_logger) << "用户的数据集路径存在，但还未有样本数";
            return false;
        }
        else if (query_result == -1)
        {
            LY_LOG_ERROR(g_logger) << "用户的数据集路径查询失败";
            return false;
        }
        return false;
    }

    bool user_db_handle::create_table(const std::string &userID, const std::string &tablename, const std::vector<std::string> &table_clo_names)
    {
        std::string sql = create_sql(tablename, std::map<std::string, std::string>{
            // {userID, "VARCHAR(20)"},
            {"age", "VARCHAR(5)"},
            {"gender", "CHAR(1)"},
            {"dress", "VARCHAR(10)"},
            {"wind_speed", "FLOAT"},
            {"BMI", "FLOAT"},
            {"height", "FLOAT"},
            {"RH", "FLOAT"},
            {"in_car_temperature", "FLOAT"},
            {"PMV", "FLOAT"},
            {"PPD", "FLOAT"},
            {"M", "FLOAT"},
            {"need_temperature", "FLOAT"},
            {"wind_type", "VARCHAR(10)"},
            {"wind_scale", "VARCHAR(10)"},
            {"direction", "VARCHAR(10)"}
        });
        // LY_LOG_INFO(g_logger)<< "create table sql: " << sql;
        return this->update_mysql(sql);
    }
    bool user_db_handle::update_dataSet_path(const std::string &userID, const std::string &User_table_name)
    {
        // char str[100];
        // sprintf(str, "update User set u_database = %s where name = %s", User_table_name.c_str(), userID.c_str());
        std::string sql = this->update_sql("User", "name=" + pack_str(userID), std::map<std::string, std::string>{{"u_database", User_table_name}});
        return this->update_mysql(sql);
    }
    int user_db_handle::select_dataSet_path_exist(const std::string &userID, std::string &dataSet_table_name)
    {
        std::string sql = this->select_sql("User", "name=" + pack_str(userID), "u_database");
        int query_r = this->query_mysql(sql, m);
        if (query_r == 0)
        {
            // LY_LOG_ERROR(g_logger)<<"用户的数据集路径存在，但还未有样本数";
            return query_r;
        }
        else if (query_r > 0)
        {
            dataSet_table_name = m["u_database"];
            delete_var();
            return query_r;
        }
        else if (query_r == -1)
        {
            LY_LOG_ERROR(g_logger) << "用户的数据集路径查询失败";
            return query_r;
        }
        return query_r;
    }
    int user_db_handle::select_dataSet_sample_number(const std::string &dataSet_table_name)
    {
        std::string sql = this->select_sql(dataSet_table_name, "", "count(*)");
        int query_clo = this->query_mysql(sql, m);
        if (query_clo > 0)
        {
            return query_clo;
        }
        else if (query_clo == 0)
        {
            LY_LOG_ERROR(g_logger) << "数据集表存在,但没数据";
            return 0;
        }
        else if (query_clo == -1)
        {
            LY_LOG_ERROR(g_logger) << "数据集表查询失败";
            return -1;
        }
        return query_clo;
    }
    int user_db_handle::delete_user(const std::string &userID)
    {
        std::string sql = this->delete_sql("User", "name = '"+userID+"'");
        bool query_r = this->update_mysql(sql);
        if (query_r)
        {
            // LY_LOG_ERROR(g_logger)<<"用户的数据集路径存在，但还未有样本数";
            LY_LOG_INFO(g_logger) << "用户删除成功";
            return 0;
        }
        else if (query_r == false)
        {
            LY_LOG_ERROR(g_logger) << "用户删除失败";
            return -1;
        }
        return -1;
    }
    int user_db_handle::insert_table_data(const std::string &table, const std::map<std::string, std::string> &data)
    {
        std::string sql = insert_sql(table, data);
        bool query_r = this->update_mysql(sql);
        if (query_r)
        {
            LY_LOG_INFO(g_logger) << "数据插入成功";
            return 1;
        }
        else if (query_r == false)
        {
            LY_LOG_ERROR(g_logger) << "数据插入失败";
            return -1;
        }
        return -1;
    }
    int user_db_handle::export_dataSet_to_csv(const std::string &table, std::string csvpath)
    {
        std::string sql = select_sql(table, "", "*");
        int sample_num = query_mysql(sql, m);
        // 创建CSV文件
        csvpath = "/home/ly/lyserver_master/other/RF/csv_data" + table + ".csv";
        std::ofstream csvFile(csvpath);
        if (!csvFile.is_open())
        {
            std::cerr << "Failed to open CSV file" << std::endl;
            return -1;
        }
        else
        {
            std::string columns = "age,gender,dress,wind_speed,BMI,height,RH,in_car_temperature,PMV,PPD,M,need_temperature,The_wind_type,wind_scale,direction";
            csvFile << columns << std::endl;
            for (int i = 0; i < sample_num; ++i)
            {
                csvFile << m["age"] << ",";
                csvFile << m["gender"] << ",";
                csvFile << m["dress"] << ",";
                csvFile << m["wind_speed"] << ",";
                csvFile << m["BMI"] << ",";
                csvFile << m["height"] << ",";
                csvFile << m["RH"] << ",";
                csvFile << m["in_car_temperature"] << ",";
                csvFile << m["PMV"] << ",";
                csvFile << m["PPD"] << ",";
                csvFile << m["M"] << ",";
                csvFile << m["need_temperature"] << ",";
                csvFile << m["The_wind_type"] << ",";
                csvFile << m["wind_scale"] << ",";  
                csvFile << m["direction"] << std::endl;
            }
            csvFile.close();
            return 1;
        }
    }
}