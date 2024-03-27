/*
 * @Author: ly ly1285620755@163.com
 * @Date: 2024-01-24 17:16:57
 * @LastEditors: ly ly1285620755@163.com
 * @LastEditTime: 2024-02-08 22:31:29
 * @FilePath: /lyserver_master/other/RF/RF.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef RF_H
#define RF_H

#include <memory>
#include <map>
#include <json/json.h>
#include "log.h"
#include "python.h"
#include "algorithm"
using namespace pyApi;
namespace Random_Forest
{
    /**
     * @description: 随机森林基类
     * @return {*}
     */    
    class RF_base
    {
    public:
        Python::ptr pyobj;
        RF_base();
        virtual ~RF_base() {}
        void import_module(const std::vector<std::string> &module_path);
    };

    class RF : public RF_base
    {
    public:
        typedef std::shared_ptr<RF> ptr;
        RF();
        ~RF();
        /**
         * @description: 初始化python,
         * @return {*}
         */
        void init_python();
        /**
         * @description: 载入随机森林的模型
         * @return {*}
         */
        void load_model();
        /**
         * @description: 载入输入
         * @return {*}
         */
        void load_inputList();
        void load_inputList(const Json::Value& obj);
        void load_input(std::map<std::string, double> &input_double, std::map<std::string, std::string> &input_str);
        /**
         * @description: 运行模型
         * @return {*}
         */
        std::vector<std::string> execute_model();
        /**
         * @description: 打印数据
         * @param {PyObject*} pydata
         * @return {*}
         */
        // void print_pydata(PyObject *pydata);

    private:
        void python_exit();

        Module::ptr pyModule;
        Object::ptr pyObject;
        Class::ptr pyClass;
        std::map<std::string, PyFunction::ptr> functionArray;

        std::vector<PyObject *> RF_vect;
        std::shared_ptr<PyObject> pList_;
    };

    class RF_train : public RF_base
    {
    public:
        typedef std::shared_ptr<RF_train> ptr;
        RF_train();
        ~RF_train();
        void load_function(const std::string &function_path);
        void load_data();
        void load_data(const std::string &data_path);
        /**
         * @brief 训练模型
         * 
         * @param save_path 默认参数="/home/ly/lyserver_master/other/RF/model"
         * @param save_model_path 传出参数，具体的保存路径
         */
        void train_model(const std::string &save_path, std::string &save_model_path);
        template <typename T, typename... Args>
        std::vector<PyObject *> run_function(T &&func_name, Args &&...args)
        {
            PyObject *function_result = functionArray[func_name]->call_back(std::forward<Args>(args)...);
            if (function_result == nullptr)
            {
                std::cout << "调用" << func_name << " 出错" << '\n';
                return {};
            }
            // 先解析返回值
            PyObject *pResultTuple = nullptr;
            Py_ssize_t result_size;
            std::vector<PyObject *> function_res;
            if (PySequence_Check(function_result))
            {
                pResultTuple = PySequence_Tuple(function_result);
                if (pResultTuple == nullptr)
                {
                    // 如果PySequence_Tuple失败，处理错误
                    std::cerr << "无法将返回值转换为元组" << '\n';
                    Py_XDECREF(function_result); // 释放之前获取的返回值
                    return {};                   // 返回或者处理错误
                }
                result_size = PyTuple_Size(pResultTuple);
                std::cout << "函数" << func_name << "返回值个数: " << result_size << '\n';

                                for (Py_ssize_t i = 0; i < result_size; i++)
                {
                    PyObject *result_item = PyTuple_GetItem(pResultTuple, i);
                    function_res.push_back(result_item);
                }
            }
            else
            {
                // 如果返回值不是序列，直接将返回值添加到结果中
                function_res.push_back(function_result);
            }

            // 清理资源
            return function_res;
        }

    private:
        Module::ptr pyModule;
        std::map<std::string, PyFunction::ptr> functionArray;
        PyObject *data_csv;
    };

    

}

#endif