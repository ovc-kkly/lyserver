#include "user_db_handle.h"

namespace lyserver
{
    user_db_handle::user_db_handle(/* args */)
    {
        // std::cout << "引用计数:"<<Conn_Pool.use_count() << "地址:"<<&Conn_Pool<< "连接数量"<< Conn_Pool->getconnectionNumber()<<'\n';
    }

    user_db_handle::~user_db_handle()
    {
    }

    int user_db_handle::isExistUser(const string &userID)
    {
        char str[50];
        sprintf(str, "select * from User where name='%s'", userID.c_str());
        int query_result = this->query_mysql(string(str), m); // 把查询到的结果记录在类的成员变量m里
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
        char str[50];
        sprintf(str, "insert into User(name,password) values('%s', '%s')", userID.c_str(), password.c_str());
        printf("%s\n", str);
        bool insert_result = this->update_mysql(string(str));
        return insert_result;
    }
    int user_db_handle::query_user_info(const string &userID, const string &password)
    {
        char str[50];
        sprintf(str, "select name, password from User where name='%s'", userID.c_str());
        unordered_map<string, string> m;
        int query_result = this->query_mysql(string(str), m); // 把查询到的结果记录变量m里
        if (query_result == 1)
        {
            if (password == m["password"])
            {
                sprintf(str, "delete from User where name='%s'", userID.c_str());
                return this->update_mysql(str) ? 1 : 0;
            }
            else
            { // 密码错误
                return -2;
            }
        }
        else
        {
            cout << "查询错误或用户不存在" << __LINE__ << endl;
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
            cout << "释放结果的保存" << endl;
        }
    }
}