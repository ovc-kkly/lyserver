#ifndef USER_DB_HANDLE_H
#define USER_DB_HANDLE_H
#include "base_db_handle.h"
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
    };

}
#endif