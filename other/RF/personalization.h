#ifndef PERSONLIZATION_H
#define PERSONLIZATION_H

#include <string>
#include <memory>
#include "RF.h"
#include "user_db_handle.h"
#include "base64.h"
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
namespace Random_Forest
{
    class PersonaLization
    {
    public:
        typedef std::shared_ptr<PersonaLization> ptr;
        PersonaLization(lyserver::user_db_handle::ptr user_db_ptr);
        ~PersonaLization();
        /**
         * @brief 判断用户是否存在
         *
         * @param user_id
         * @return true
         * @return false
         */
        bool judge_user_is_exist(const std::string user_id);
        /**
         * @brief 改变用户的模型路径
         *
         * @param user_id
         * @param model_dir
         * @return true
         * @return false
         */
        bool change_user_2_modeldir(const std::string user_id, const std::string model_dir);
        /**
         * @brief 查询用户的模型路径
         *
         * @param user_id
         * @param model_dir 传出参数
         * @return std::string
         */
        void search_user_modeldir(const std::string user_id, std::string &model_dir);
        /**
         * @brief 创建用户表，用来存储数据集
         *
         */
        bool create_user_dataSet_table(const std::string user_id);

        /**
         * @brief 决策用户该使用哪个模型
         *
         * @param user_id
         */
        void decide_userid_modelpath(const std::string user_id);
        void decide_u_modelpath(const std::string user_id, const std::string model_path);
        /**
         * @brief Create a loacal model dir
         *
         * @param user_id
         * @return true
         * @return false
         */
        bool create_loacal_model_dir(const std::string user_id, std::string& model_dir_path);
        /**
         * @brief 删除数据库中的用户信息
         * 
         * @param user_id 
         * @return int 
         */
        int clear(const std::string user_id);
        /**
         * @brief 向用户数据集表中插入一条数据
         * 
         * @param user_id 
         * @param one_dataSet 
         * @return true 
         * @return false 
         */
        bool insert_userid_one_dataSet(const std::string user_id, const std::map<std::string, std::string> one_dataSet);
        /**
         * @brief 训练数据集
         * 
         * @param dataset_csv 
         * @return std::string 
         */
        std::string train_dataset(const std::string dataset_csv);
        void remove_equals_sign(std::string& str);
    private:
        lyserver::user_db_handle::ptr user_db_ptr_;
        std::map<std::string, std::string> userID_modelpath;//当前用户ID对应的模型文件路径
    };
}

#endif