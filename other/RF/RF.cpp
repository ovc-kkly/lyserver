#include "RF.h"

namespace Random_Forest
{
    static lyserver::Logger::ptr ly_logger = LY_LOG_NAME("ly");
    RF_base::RF_base()
    {
        pyobj = PyMgr::GetInstance()->getPython_ptr();
        pyobj->run("import sys");
    }
    void RF_base::import_module(const std::vector<std::string> &module_path)
    {
        for (auto &path : module_path)
        {
            pyobj->run(path);
        }
    }
    RF::RF()
    {
        init_python();
        load_model();
    }
    RF::~RF()
    {
        python_exit();
    }
    void RF::init_python()
    {
        // pyobj.reset(new Python);
        pyobj->run("sys.path.append('/home/ly/lyserver_master/other/RF/script')");
        pyModule.reset(new Module);
        pyModule->import("RF_predict");

        PyFunction::ptr func1(new PyFunction(*pyModule, "load_model_and_vect"));
        PyFunction::ptr func2(new PyFunction(*pyModule, "predict_label"));
        if (!func1->get_func())
        {
            LY_LOG_ERROR(ly_logger) << "func1 is nullptr";
        }
        else
        {
            functionArray["func_loadmodel"] = func1;
            LY_LOG_INFO(ly_logger) << "load function [load_model_and_vect]";
        }
        if (!func2->get_func())
        {
            LY_LOG_ERROR(ly_logger) << "func2 is nullptr";
        }
        else
        {
            functionArray["func_predict"] = func2;
            LY_LOG_INFO(ly_logger) << "load function [predict_label]";
        }
    }
    void RF::python_exit()
    {
        for (auto &obj : RF_vect)
        {
            if (obj)
            {
                Py_XDECREF(obj);
            }
        }
    }
    void RF::load_input(std::map<std::string, double> &input_double, std::map<std::string, std::string> &input_str)
    {

        // Example input data (in_list)
        input_str["age"] = "青年";
        input_str["gender"] = "女";
        input_double["BMI"] = 17.7;
        input_str["dress"] = "春秋装";
        input_double["in-car temperature"] = 24.05;
        input_double["RH"] = 56.12;
        input_double["height(m)"] = 1.53;
        input_double["wind_speed"] = 0.5;
    }
    void RF::load_model()
    {
        PyObject *pResultLoadModel = functionArray["func_loadmodel"]->call_back(PyUnicode_DecodeFSDefault("/home/ly/lyserver_master/other/RF/model/general_model/all_model.pkl"), PyUnicode_DecodeFSDefault("/home/ly/lyserver_master/other/RF/model/general_model/all_vect.pkl"));
        if (pResultLoadModel == nullptr)
        {
            PyErr_Print();
            LY_LOG_ERROR(ly_logger) << "调用 load_model_and_vect 函数错误";
            return;
        }
        PyObject *pResultTuple = PySequence_Tuple(pResultLoadModel);

        RF_vect.push_back(PyTuple_GetItem(pResultTuple, 0));
        RF_vect.push_back(PyTuple_GetItem(pResultTuple, 1));
        LY_LOG_INFO(ly_logger) << "load model [RF vect]";
    }
    void RF::load_inputList()
    {
        std::map<std::string, double> input_double;
        std::map<std::string, std::string> input_str;
        load_input(input_double, input_str);

        // Create a Python list for input data
        PyObject *pList = PyList_New(0);
        PyList_Append(pList, PyUnicode_DecodeFSDefault(input_str["age"].c_str()));
        PyList_Append(pList, PyUnicode_DecodeFSDefault(input_str["gender"].c_str()));
        PyList_Append(pList, PyFloat_FromDouble(input_double["BMI"]));
        PyList_Append(pList, PyUnicode_DecodeFSDefault(input_str["dress"].c_str()));
        PyList_Append(pList, PyFloat_FromDouble(input_double["in_car_temperature"]));
        PyList_Append(pList, PyFloat_FromDouble(input_double["RH"]));
        PyList_Append(pList, PyFloat_FromDouble(input_double["height"]));
        PyList_Append(pList, PyFloat_FromDouble(input_double["wind_speed"]));

        // Call the predict_label function
        pList_.reset(PyList_New(0), [](PyObject *obj)
                     {
            if(obj){
                Py_XDECREF(obj);
            } });
        PyList_Append(pList_.get(), pList);
        // Py_ssize_t listSize = PyList_Size(pList_.get());
        // Py_ssize_t listSize_ = PyList_Size(pList);
        // LY_LOG_INFO(ly_logger) << "New List size: " << listSize << listSize_;
    }
    void RF::load_inputList(const Json::Value& obj)
    {
        std::map<std::string, double> input_double;
        std::map<std::string, std::string> input_str;

        input_str["age"] = obj["age"].asString();
        input_str["gender"] = obj["gender"].asString();
        input_str["dress"] = obj["dress"].asString();
        input_double["BMI"] = obj["BMI"].asDouble();
        input_double["in_car_temp"] = obj["in_car_temp"].asDouble();
        input_double["RH"] = obj["RH"].asDouble();
        input_double["height"] = obj["height"].asDouble();
        input_str["wind_speed"] = obj["wind_speed"].asString();

        // Create a Python list for input data
        PyObject *pList = PyList_New(0);
        PyList_Append(pList, PyUnicode_DecodeFSDefault(input_str["age"].c_str()));
        PyList_Append(pList, PyUnicode_DecodeFSDefault(input_str["gender"].c_str()));
        PyList_Append(pList, PyFloat_FromDouble(input_double["BMI"]));
        PyList_Append(pList, PyUnicode_DecodeFSDefault(input_str["dress"].c_str()));
        PyList_Append(pList, PyFloat_FromDouble(input_double["in_car_temp"]));
        PyList_Append(pList, PyFloat_FromDouble(input_double["RH"]));
        PyList_Append(pList, PyFloat_FromDouble(input_double["height"]));
        PyList_Append(pList, PyUnicode_DecodeFSDefault(input_str["wind_speed"].c_str()));

        pList_.reset(PyList_New(0), [](PyObject *p){
            if(p){
                Py_XDECREF(p);
            }
        });
        PyList_Append(pList_.get(), pList);
    }

    std::vector<std::string> RF::execute_model()
    {
        // print_pydata(pList_.get());
        PyObject *pResultPredict = functionArray["func_predict"]->call_back(pList_.get(), RF_vect[0], RF_vect[1]);
        if (pResultPredict == nullptr)
        {
            PyErr_Print();
            LY_LOG_ERROR(ly_logger) << "调用 predict_label 函数错误";
            return {};
        }
        std::vector<std::string> res;
        pyobj->print_pydata(pResultPredict, res);
        return res;
    }
    // void RF::print_pydata(PyObject *pydata)
    // {
    //     // 判断对象类型
    //     if (PyList_Check(pydata))
    //     {
    //         // 处理列表
    //         Py_ssize_t size = PyList_Size(pydata);
    //         for (Py_ssize_t i = 0; i < size; ++i)
    //         {
    //             PyObject *item = PyList_GetItem(pydata, i);
    //             print_pydata(item);
    //         }
    //     }
    //     else if (PyDict_Check(pydata))
    //     {
    //         // 处理字典
    //         PyObject *key, *value;
    //         Py_ssize_t pos = 0;
    //         while (PyDict_Next(pydata, &pos, &key, &value))
    //         {
    //             print_pydata(value);
    //         }
    //     }
    //     else if (PyUnicode_Check(pydata))
    //     {
    //         // 处理字符串
    //         printf("%s\n", PyUnicode_AsUTF8(pydata));
    //     }
    //     else if (PyLong_Check(pydata) || PyFloat_Check(pydata))
    //     {
    //         // 处理数字
    //         printf("%g\n", PyFloat_AsDouble(pydata));
    //     }
    //     else
    //     {
    //         // 未知类型
    //         printf("Unknown Type\n");
    //     }
    // }

    RF_train::RF_train()
    {
    }
    RF_train::~RF_train()
    {
    }
    void RF_train::load_data()
    {
        data_csv = functionArray["get_data"]->call_back(PyUnicode_DecodeFSDefault("/home/ly/lyserver_master/other/RF/csv_data/dataset_pmv_need_temp.csv"));
        if (data_csv == nullptr)
        {
            PyErr_Print();
            LY_LOG_ERROR(ly_logger) << "调用 get_data 函数错误";
            return;
        }
        else
        {
            LY_LOG_INFO(ly_logger) << "load csv data success";
        }
    }
    void RF_train::load_data(const std::string &data_path)
    {
        data_csv = functionArray["get_data"]->call_back(data_path);
        if (data_csv == nullptr)
        {
            PyErr_Print();
            LY_LOG_ERROR(ly_logger) << "调用 get_data 函数错误";
            return;
        }
        else
        {
            LY_LOG_INFO(ly_logger) << "load csv data success";
        }
    }

    void RF_train::load_function(const std::string &function_path)
    {
        /**
         *
         */
        pyobj->run(function_path);
        // pyobj->run("sys.path.append('/home/ly/lyserver_master/other/RF/script')");
        pyModule.reset(new Module);
        pyModule->import("plus_RF");

        PyFunction::ptr func1(new PyFunction(*pyModule, "get_data"));
        PyFunction::ptr func2(new PyFunction(*pyModule, "mul_target_encoder"));
        PyFunction::ptr func3(new PyFunction(*pyModule, "data_deal"));
        PyFunction::ptr func4(new PyFunction(*pyModule, "feature_extraction"));
        PyFunction::ptr func5(new PyFunction(*pyModule, "check_model_xinneng"));
        PyFunction::ptr func6(new PyFunction(*pyModule, "split_data"));
        PyFunction::ptr func7(new PyFunction(*pyModule, "main_fun"));
        PyFunction::ptr func8(new PyFunction(*pyModule, "rf_predict"));
        PyFunction::ptr func9(new PyFunction(*pyModule, "save_model"));

        if (!func1->get_func())
        {
            LY_LOG_ERROR(ly_logger) << "func1 is nullptr";
        }
        else
        {
            functionArray["get_data"] = func1;
            LY_LOG_INFO(ly_logger) << "load function [get_data]";
        }
        if (!func2->get_func())
        {
            LY_LOG_ERROR(ly_logger) << "func2 is nullptr";
        }
        else
        {
            functionArray["mul_target_encoder"] = func2;
            LY_LOG_INFO(ly_logger) << "load function [mul_target_encoder]";
        }
        if (!func3->get_func())
        {
            LY_LOG_ERROR(ly_logger) << "func3 is nullptr";
        }
        else
        {
            functionArray["data_deal"] = func3;
            LY_LOG_INFO(ly_logger) << "load function [data_deal]";
        }
        if (!func4->get_func())
        {
            LY_LOG_ERROR(ly_logger) << "func4 is nullptr";
        }
        else
        {
            functionArray["feature_extraction"] = func4;
            LY_LOG_INFO(ly_logger) << "load function [feature_extraction]";
        }
        if (!func5->get_func())
        {
            LY_LOG_ERROR(ly_logger) << "func5 is nullptr";
        }
        else
        {
            functionArray["check_model_xinneng"] = func5;
            LY_LOG_INFO(ly_logger) << "load function [check_model_xinneng]";
        }
        if (!func6->get_func())
        {
            LY_LOG_ERROR(ly_logger) << "func6 is nullptr";
        }
        else
        {
            functionArray["split_data"] = func6;
            LY_LOG_INFO(ly_logger) << "load function [split_data]";
        }
        if (!func7->get_func())
        {
            LY_LOG_ERROR(ly_logger) << "func7 is nullptr";
        }
        else
        {
            functionArray["main_fun"] = func7;
            LY_LOG_INFO(ly_logger) << "load function [main_fun]";
        }
        if (!func8->get_func())
        {
            LY_LOG_ERROR(ly_logger) << "func8 is nullptr";
        }
        else
        {
            functionArray["rf_predict"] = func8;
            LY_LOG_INFO(ly_logger) << "load function [rf_predict]";
        }
        if (!func9->get_func())
        {
            LY_LOG_ERROR(ly_logger) << "func9 is nullptr";
        }
        else
        {
            functionArray["save_model"] = func9;
            LY_LOG_INFO(ly_logger) << "load function [save_model]";
        }
    }

    void RF_train::train_model(const std::string &save_path, std::string &save_model_path)
    {

        // data_features, data_targets, data_predict_features, data_predict_targets
        std::vector<PyObject *> data_deal_res = run_function<std::string>("data_deal", data_csv);
        if (0 == data_deal_res.size())
        {
            LY_LOG_INFO(ly_logger) << "train model fail in data_deal";
        }
        // data_featured, data_predict_featured, vect
        std::vector<PyObject *> feature_extraction_res = run_function<std::string>("feature_extraction", data_deal_res[0], data_deal_res[2]);
        if (0 == feature_extraction_res.size())
        {
            LY_LOG_INFO(ly_logger) << "train model fail in feature_extraction";
        }
        // rf
        std::vector<PyObject *> split_data_res = run_function<std::string>("split_data", feature_extraction_res[0], data_deal_res[1]);
        if (0 == split_data_res.size())
        {
            LY_LOG_INFO(ly_logger) << "train model fail in split_data";
        }
        // result
        std::vector<PyObject *> rf_predict_res = run_function<std::string>("rf_predict", split_data_res[0], feature_extraction_res[1]);
        if (0 == rf_predict_res.size())
        {
            LY_LOG_INFO(ly_logger) << "train model fail in rf_predict";
        }
        // joblib
        std::vector<std::string> Files;
        lyserver::FSUtil::ListAllFile(Files, save_path, "");

        size_t index = 0;
        std::string md;
        for (; index < Files.size(); index++)
        {
            md = "model_" + std::to_string(index);
            auto it = std::find_if(Files.begin(), Files.end(), [&](const std::string &file)
                                   { return file == md; });
            if (it == Files.end())
            {
                break;
            }
        }

        save_model_path = save_path + "/" + md;
        std::string all_model_path = save_model_path + "/all_model.pkl";
        std::string all_vect_path = save_model_path + "/all_vect.pkl";
        lyserver::FSUtil::Mkdir(save_model_path);
        std::vector<PyObject *> save_model_res = run_function<std::string>("save_model", split_data_res[0], feature_extraction_res[2],
                                                                      PyUnicode_DecodeFSDefault(all_model_path.c_str()),
                                                                      PyUnicode_DecodeFSDefault(all_vect_path.c_str()));
        if (0 == save_model_res.size())
        {
            LY_LOG_INFO(ly_logger) << "train model fail in save_model";
        }

        LY_LOG_INFO(ly_logger) << "train model success";
    }
}