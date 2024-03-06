/*
 * @Author: ly ly1285620755@163.com
 * @Date: 2024-02-06 14:51:39
 * @LastEditors: ly ly1285620755@163.com
 * @LastEditTime: 2024-02-08 13:02:06
 * @FilePath: /lyserver_master/other/py/python.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "python.h"
namespace pyApi
{
    Python* Python::pyobj=nullptr;
    Python::Python()
    {
        Py_Initialize();
        if (!Py_IsInitialized())
        {
            throw std::logic_error("Python init failed");
        }
    }
    Python::~Python()
    {
        Py_Finalize();
    }
    void Python::run(const std::string &str)
    {
        PyRun_SimpleString(str.c_str());
    }

    Argument::~Argument()
    {
        Py_XDECREF(m_args);
    }

    Module::Module():m_module(nullptr)
    {
    }
    Module::Module(const std::string &name):Module()
    {
        m_module = PyImport_ImportModule(name.c_str());
    }
    Module::~Module()
    {
        Py_XDECREF(m_module);
    }
    void Module::import(const std::string &name)
    {
        m_module = PyImport_ImportModule(name.c_str());
    }

    Class::Class(const Module &module, const std::string &name)
    {
        m_class = PyObject_GetAttrString(module.m_module, name.c_str());
        if (!m_class)
        {
            std::cout << "class not found: Person" << std::endl;
        }
    }
    Class::~Class()
    {
        Py_XDECREF(m_class);
    }

    Object::Object(const Class &cls)
    {
        m_object = PyEval_CallObject(cls.m_class, nullptr);
    }
    Object::~Object()
    {
        Py_XDECREF(m_object);
    }

    PyFunction::PyFunction(const Module &module, const std::string &name)
    {
        if(!module.m_module){
            std::cout << "module is null"<< std::endl;
            return;
        }
        m_func = PyObject_GetAttrString(module.m_module, name.c_str());
        if (!m_func || !PyCallable_Check(m_func))
        {
            std::cout << "function not found: say" << std::endl;
        }
    }
    PyFunction::PyFunction(const Object &object, const std::string &name)
    {
        m_func = PyObject_GetAttrString(object.m_object, name.c_str());
    }
    PyFunction::~PyFunction()
    {
        Py_XDECREF(m_func);
    }
    void PyFunction::call()
    {
        PyObject_CallObject(m_func, nullptr);
    }

    void Python::print_pydata(PyObject *pydata, std::vector<std::string> &res)
    {
        // 判断对象类型
        if (PyList_Check(pydata))
        {
            // 处理列表
            Py_ssize_t size = PyList_Size(pydata);
            for (Py_ssize_t i = 0; i < size; ++i)
            {
                PyObject *item = PyList_GetItem(pydata, i);
                print_pydata(item, res);
            }
        }
        else if (PyDict_Check(pydata))
        {
            // 处理字典
            PyObject *key, *value;
            Py_ssize_t pos = 0;
            while (PyDict_Next(pydata, &pos, &key, &value))
            {
                print_pydata(value, res);
            }
        }
        else if (PyUnicode_Check(pydata))
        {
            // 处理字符串
            printf("%s\n", PyUnicode_AsUTF8(pydata));
            res.push_back(std::string(PyUnicode_AsUTF8(pydata)));
        }
        else if (PyLong_Check(pydata) || PyFloat_Check(pydata))
        {
            // 处理数字
            printf("%g\n", PyFloat_AsDouble(pydata));
            res.push_back(std::to_string(PyFloat_AsDouble(pydata)));
        }
        else
        {
            // 未知类型
            printf("Unknown Type\n");
        }
    }
}