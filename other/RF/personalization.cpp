/*
 * @Author: ly ly1285620755@163.com
 * @Date: 2024-02-08 17:58:03
 * @LastEditors: ly ly1285620755@163.com
 * @LastEditTime: 2024-02-09 02:40:43
 * @FilePath: /lyserver_master/other/RF/personalization.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "personalization.h"

namespace Random_Forest
{
    static lyserver::Logger::ptr ly_logger = LY_LOG_NAME("ly");
    PersonaLization::PersonaLization(lyserver::user_db_handle::ptr user_db_ptr) : user_db_ptr_(user_db_ptr)
    {
    }
    PersonaLization::~PersonaLization()
    {
    }
    void PersonaLization::remove_equals_sign(std::string& str)
    {
        size_t pos = str.find('=');
        while (pos != std::string::npos) {
            str.erase(pos, 1); // 移除等号
            pos = str.find('=', pos); // 更新查找位置
        }
    }
    bool PersonaLization::judge_user_is_exist(const std::string user_id)
    {
        int ret_ = user_db_ptr_->isExistUser(user_id);
        if (ret_ == -1)
        {
            LY_LOG_WARN(ly_logger) << "查询错误";
        }
        else if (ret_ == 0)
        {
            LY_LOG_WARN(ly_logger) << "查询用户不存在";
        }
        else
        {
            LY_LOG_INFO(ly_logger) << "用户存在";
        }
        return ret_ > 0 ? true : false;
    }
    bool PersonaLization::change_user_2_modeldir(const std::string user_id, const std::string model_dir)
    {
        bool ret = this->judge_user_is_exist(user_id);
        if (ret)
        {
            if (user_db_ptr_->update_modeldir(user_id, model_dir))
            {
                LY_LOG_INFO(ly_logger) << "更新用户模型路径成功";
                return true;
            }
            else
            {
                LY_LOG_WARN(ly_logger) << "更新用户模型路径失败";
            }
            return false;
        }
        return false;
    }
    void PersonaLization::search_user_modeldir(const std::string user_id, std::string &model_dir)
    {
        bool ret = this->judge_user_is_exist(user_id);
        if (ret)
        {
            if (user_db_ptr_->select_modeldir(user_id, model_dir))
            {
                LY_LOG_INFO(ly_logger) << "查询成功，得到模型路径";
            }
            else
            {
                LY_LOG_WARN(ly_logger) << "查询模型路径失败";
            }
        }
    }
    bool PersonaLization::create_user_dataSet_table(const std::string user_id)
    {
        std::string user_tableName = lyserver::base64_encode(user_id.c_str(), user_id.size());
        remove_equals_sign(user_tableName);
        std::string user_tableName_ = "user" + user_tableName;
        if (user_db_ptr_->create_table(user_id, user_tableName_, {}))
        {
            LY_LOG_INFO(ly_logger) << "创建用户表成功";
            // 更新数据集路径（表名）
            return user_db_ptr_->update_dataSet_path(user_id, user_tableName_);
        }
        else
        {
            LY_LOG_WARN(ly_logger) << "创建用户表失败";
        }
        return false;
    }

    void PersonaLization::decide_userid_modelpath(const std::string user_id)
    {
        std::string user_dataset_table_name;
        int dataset = user_db_ptr_->select_dataSet_path_exist(user_id, user_dataset_table_name);
        if (dataset)
        {
            LY_LOG_INFO(ly_logger) << "数据集表名：" << user_dataset_table_name;
            int dataSet_samplenum = user_db_ptr_->select_dataSet_sample_number(user_dataset_table_name);
            if (dataSet_samplenum == 0)
            {
                LY_LOG_WARN(ly_logger) << "用户数据集为空，请添加数据集样本数";
            }
            else if (dataSet_samplenum < 100 && dataSet_samplenum > 0)
            {
                userID_modelpath[user_id] = "/home/ly/lyserver_master/other/RF/model/general_model";
                LY_LOG_INFO(ly_logger) << "用户数据集样本数大于0但小于100，使用通用模型";
            }
            else if (dataSet_samplenum >= 100 && dataSet_samplenum < 500)
            {
                this->search_user_modeldir(user_id, userID_modelpath[user_id]);
                LY_LOG_INFO(ly_logger) << "用户数据集样本数大于等于100小于500，使用该ID对应的模型";
            }
            else if (dataSet_samplenum >= 500 && dataSet_samplenum < 1000)
            {
                this->search_user_modeldir(user_id, userID_modelpath[user_id]);
                LY_LOG_INFO(ly_logger) << "用户数据集样本数大于等于500小于1000，使用该ID对应的模型";
            }
            else if (dataSet_samplenum >= 1000 && dataSet_samplenum < 2000)
            {
                this->search_user_modeldir(user_id, userID_modelpath[user_id]);
                LY_LOG_INFO(ly_logger) << "用户数据集样本数大于等于1000小于2000，使用该ID对应的模型";
            }
        }
        else if (dataset == 0)
        {
            LY_LOG_WARN(ly_logger) << "用户数据集路径存在,还没有数据";
        }
        else if (dataset == -1)
        {
            LY_LOG_WARN(ly_logger) << "用户不存在数据集路径";
        }
    }
    void PersonaLization::decide_u_modelpath(const std::string user_id, const std::string model_path)
    {
        userID_modelpath[user_id] = model_path;
    }
    bool PersonaLization::create_loacal_model_dir(const std::string user_id, std::string &model_dir_path)
    {
        std::string model_dir = lyserver::base64_encode(user_id.c_str(), user_id.size());
        model_dir = "model_" + model_dir;
        model_dir_path = "/home/ly/lyserver_master/other/RF/model/" + model_dir;

        const char *folderPath = model_dir_path.c_str();
        int ret = mkdir(folderPath, S_IRWXU | S_IRWXG | S_IRWXO);
        if (ret)
        {
            // 创建成功
            LY_LOG_INFO(ly_logger) << "创建模型文件夹成功";
            return true;
        }
        else if (ret == -1)
        {
            // 创建失败，处理错误
            std::string error_message = "创建模型文件夹失败: ";
            switch (errno)
            {
            case EEXIST:
                error_message += "目录已存在";
                break;
            case EACCES:
                error_message += "没有足够的权限";
                break;
            case ENOMEM:
                error_message += "内存不足";
                break;
            case ENOSPC:
                error_message += "磁盘空间不足";
                break;
            case ENOTDIR:
                error_message += "路径中的某个部分不是目录";
                break;
            case EROFS:
                error_message += "文件系统是只读的";
                break;
            case ELOOP:
                error_message += "路径中存在过多的符号链接";
                break;
            case EIO:
                error_message += "I/O错误";
                break;
            case EFAULT:
                error_message += "路径名无效";
                break;
            default:
                error_message += "未知错误";
                break;
            }
            LY_LOG_ERROR(ly_logger) << error_message;
            return false;
        }
        return true;
    }
    int PersonaLization::clear(const std::string user_id)
    {
        user_db_ptr_->delete_user(user_id);
        return this->userID_modelpath.erase(user_id);
    }
    bool PersonaLization::insert_userid_one_dataSet(const std::string user_id, const std::map<std::string, std::string> one_dataSet)
    {
        std::string dataSet_tablename;
        // 查询数据集路径表是否存在
        user_db_ptr_->select_dataSet_path_exist(user_id, dataSet_tablename);

        if (user_db_ptr_->insert_table_data(dataSet_tablename, one_dataSet))
        {
            int table_sample_number = user_db_ptr_->select_dataSet_sample_number(dataSet_tablename);
            /**
             * @brief 插完数据，开始判断数据集是否满足阈值要求，并训练
             *
             */
            if (table_sample_number == 100 || table_sample_number == 500 || table_sample_number == 1000 || table_sample_number == 2000)
            { // 来到这里代表到达指定阈值，要训练数据集
                std::string csvpath;
                user_db_ptr_->export_dataSet_to_csv(dataSet_tablename, csvpath);
                std::string model_path = this->train_dataset(csvpath);
                user_db_ptr_->update_modeldir(user_id, model_path);
                decide_u_modelpath(user_id, model_path);
            }
            return true;
        }
        else
        {
            return false;
        }
    }
    std::string PersonaLization::train_dataset(const std::string dataset_csv)
    {
        using namespace Random_Forest;
        RF_train::ptr RFT(new RF_train);

        std::vector<std::string> import_statements;

        // 添加导入语句到vector
        import_statements.push_back("import numpy as np");
        import_statements.push_back("import pandas as pd");
        import_statements.push_back("from sklearn.ensemble import RandomForestClassifier");
        import_statements.push_back("from sklearn.model_selection import train_test_split");
        import_statements.push_back("from sklearn.feature_extraction import DictVectorizer");
        import_statements.push_back("from sklearn.model_selection import cross_val_score, GridSearchCV");
        import_statements.push_back("from sklearn.metrics import accuracy_score, precision_score, f1_score, recall_score");
        import_statements.push_back("from sklearn.multioutput import MultiOutputClassifier");
        import_statements.push_back("from sklearn.pipeline import Pipeline");
        import_statements.push_back("import joblib");
        RFT->import_module(import_statements);
        RFT->load_function("sys.path.append('/home/ly/lyserver_master/other/RF/script')");
        RFT->load_data(dataset_csv);
        std::string save_model_path;
        RFT->train_model("/home/ly/lyserver_master/other/RF/model/", save_model_path);
        return save_model_path;
    }
}
