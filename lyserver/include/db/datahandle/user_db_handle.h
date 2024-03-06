#ifndef USER_DB_HANDLE_H
#define USER_DB_HANDLE_H
#include "base_db_handle.h"
#include <fstream>
using namespace std;
namespace lyserver
{
    class user_db_handle : public Db_handle
    {
    private:
        /* data */
        /**
         * @brief 查询结果保存变量
         *
         */
        std::unordered_map<std::string, std::string> m;
        /**
         * @brief 保存已经登录的账号
         *
         */
        std::unordered_map<std::string, bool> User_ID;

    public:
        typedef std::shared_ptr<user_db_handle> ptr;
        // using Db_handle::Db_handle;//直接继承父类的所有的构造函数
        user_db_handle(/* args */);
        ~user_db_handle();
        /*
         *@判断用户是否存在
         */
        int isExistUser(const std::string &userID);
        /*
         *@判断密码是否对错
         */
        bool passward_right_wrong(const std::string &password);
        /*
         *@插入用户数据
         */
        bool insert_userID(const std::string &userID, const std::string &password);
        /**
         * @brief 查询用户信息
         *
         * @param userID
         * @param password
         * @return int
         */
        int query_user_info(const std::string &userID, const std::string &password);
        /**
         * @brief 改变用户的登录状态
         *
         * @param User_name
         * @param on_off
         */
        void change_user_log_on_state(const std::string &User_name, bool on_off);
        /**
         * @brief 清空变量m
         *
         */
        void delete_var();
        /**
         * @brief 更新用户对应的模型路径
         *
         * @param userID
         * @param modeldir
         */
        bool update_modeldir(const std::string &userID, const std::string &modeldir);
        /**
         * @brief 查询模型路径
         *
         * @param userID
         * @param modeldir 传出参数
         * @return true
         * @return false
         */
        bool select_modeldir(const std::string &userID, std::string &modeldir);
        /**
         * @brief Create a table object
         *
         * @param userID
         * @param tablename
         * @param table_clo_names
         * @return true
         * @return false
         */
        bool create_table(const std::string &userID, const std::string &tablename, const std::vector<std::string> &table_clo_names);
        /**
         * @brief 更新用户数据集路径
         *
         * @param userID
         * @param User_table_name
         * @return true
         * @return false
         */
        bool update_dataSet_path(const std::string &userID, const std::string &User_table_name);
        /**
         * @brief 查询数据集路径
         *
         * @param userID
         * @return > 0 : 数据集路径存在，返回样本数
         * @return == 0 ： 还没有样本数，样本数0
         * @return == -1: 不存在数据集路径
         */
        int select_dataSet_path_exist(const std::string &userID, std::string &dataSet_table_name);
        /**
         * @brief 查询数据集表的行数，即样本数
         *
         * @param dataSet_table_name
         * @return int > 0: 样本数
         *              == 0 : 有该数据集表，但没有数据
         *              == -1 : 查询失败
         */
        int select_dataSet_sample_number(const std::string &dataSet_table_name);
        /**
         * @brief 删除用户信息,没删除了用户数据集表
         * 
         * @param userID 
         * @return int 
         */
        int delete_user(const std::string &userID);

        /**
         * @brief 向表插入一条样本数据
         * 
         * @param table 
         * @param data 
         * @return int 
         */
        int insert_table_data(const std::string &table, const std::map<std::string, std::string> &data);

        int export_dataSet_to_csv(const std::string &table, std::string csvpath);
    };

}
#endif