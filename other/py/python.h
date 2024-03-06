#ifndef PYTHON_H
#define PYTHON_H
#include <string>
#include <python3.10/Python.h>
#include <stdexcept>
#include <memory>
#include <iostream>
#include "mutex.h"
#include "singleton.h"
namespace pyApi
{
    class PythonManager;
    class Python
    {
        friend class PythonManager;
    public:
        typedef std::shared_ptr<Python> ptr;
        Python();
        ~Python();

        void run(const std::string &str);
        void print_pydata(PyObject *pydata, std::vector<std::string> &res);

    private:
        static Python *pyobj;
    };
    class PythonManager
    {
    public:
        Python::ptr getPython_ptr()
        {
            // 获取单例实例的全局访问点
            // 使用双检锁（Double-Checked Locking）确保线程安全
            if (Python::pyobj == nullptr)
            {
                lyserver::RWMutex::WriteLock lock(mutex);
                if (Python::pyobj == nullptr)
                {
                    Python::pyobj = new Python;
                }
            }
            return Python::ptr(Python::pyobj);
        }

    private:
        lyserver::RWMutex mutex;
    };
    typedef lyserver::Singleton<PythonManager> PyMgr;

    /**=======================================================================================================================================================
     * argument===============================================================================================================================================
     */
    class Object;
    class PyFunction;
    class Argument
    {
        friend class Object;
        friend class PyFunction;

    public:
        Argument() : m_args(nullptr), m_num(0), m_pos(0){};
        ~Argument();
        template <typename... Args>
        void bind(Args... args);

        template <typename R>
        R parse_result(PyObject *ret);

        PyObject *get_args() { return m_args; }
        int get_num() { return m_num; }

    protected:
        void bind_inner() {}
        template <typename T, typename... Args>
        void bind_inner(T arg, Args... args);
        template <typename T>
        std::string data_flag();

    private:
        PyObject *m_args = nullptr;
        int m_num;
        int m_pos;
    };

    template <typename... Args>
    void Argument::bind(Args... args)
    {
        m_num = sizeof...(Args);
        m_pos = 0;
        m_args = PyTuple_New(m_num);
        bind_inner(args...);
    }

    template <typename R>
    R Argument::parse_result(PyObject *ret)
    {

        R result;
        std::string flag = data_flag<R>();
        int parse_result = PyArg_Parse(ret, flag.c_str(), &result);

        // 处理解析失败的情况
        if (!parse_result)
        {
            // 根据需要抛出异常或者返回默认值
            throw std::runtime_error("Failed to parse PyObject to desired type");
        }

        return result;
    }

    template <typename T, typename... Args>
    void Argument::bind_inner(T arg, Args... args)
    {
        std::string flag = data_flag<T>();
        PyObject *val = Py_BuildValue(flag.c_str(), arg);
        PyTuple_SetItem(m_args, m_pos++, val);
        bind_inner(args...);
    }

    template <typename T>
    std::string Argument::data_flag()
    {
        std::string flag;
        if (typeid(T) == typeid(bool))
        {
            flag = "b";
        }
        else if (typeid(T) == typeid(int))
        {
            flag = "i";
        }
        else if (typeid(T) == typeid(unsigned int))
        {
            flag = "I";
        }
        else if (typeid(T) == typeid(float))
        {
            flag = "f";
        }
        else if (typeid(T) == typeid(double))
        {
            flag = "d";
        }
        else if (typeid(T) == typeid(const char *))
        {
            flag = "s";
        }
        return flag;
    }
    /**=======================================================================================================================================================
     * Module=================================================================================================================================================
     */
    class Module
    {
        friend class Class;
        friend class PyFunction;

    public:
        typedef std::shared_ptr<Module> ptr;
        Module();
        Module(const std::string &name);
        ~Module();
        void import(const std::string &name);
        PyObject *get_module()
        {
            return m_module;
        }

    private:
        PyObject *m_module = nullptr;
    };

    /**=======================================================================================================================================================
     * pyclass=================================================================================================================================================
     */
    class Object;
    class Class
    {
        friend class Object;

    public:
        typedef std::shared_ptr<Class> ptr;
        Class() = default;
        Class(const Module &module, const std::string &name);
        ~Class();
        PyObject *get_class()
        {
            return m_class;
        }

    private:
        PyObject *m_class = nullptr;
    };

    /**=======================================================================================================================================================
     * Object=================================================================================================================================================
     */
    class Object
    {
        friend class PyFunction;

    public:
        typedef std::shared_ptr<Object> ptr;
        Object() = default;
        Object(const Class &cls);
        template <typename... Args>
        Object(const Class &cls, Args... args);
        ~Object();
        PyObject *get_object()
        {
            return m_object;
        }

    private:
        PyObject *m_object = nullptr;
    };

    template <typename... Args>
    Object::Object(const Class &cls, Args... args)
    {
        auto arg = Argument();
        arg.bind(args...);
        m_object = PyEval_CallObject(cls.m_class, arg.m_args);
    }

    /**=======================================================================================================================================================
     * pyfunction=================================================================================================================================================
     */
    class PyFunction
    {
    public:
        typedef std::shared_ptr<PyFunction> ptr;
        PyFunction() = default;
        PyFunction(const Module &module, const std::string &name);
        PyFunction(const Object &object, const std::string &name);
        ~PyFunction();
        void call();
        template <typename... Args>
        PyObject *call_back(Args... args);
        template <typename R>
        R call();

        template <typename R, typename... Args>
        R call(Args... args);

        PyObject *get_func() { return m_func; }

    private:
        PyObject *m_func = nullptr;
    };

    template <typename R>
    R PyFunction::call()
    {
        auto arg = Argument();
        PyObject *ret = PyObject_CallObject(m_func, arg.m_args);
        return arg.parse_result<R>(ret);
    }

    template <typename R, typename... Args>
    R PyFunction::call(Args... args)
    {
        Argument arg;
        arg.bind(args...);
        PyObject *ret = PyObject_CallObject(m_func, arg.m_args);
        return arg.parse_result<R>(ret);
    }
    template <typename... Args>
    PyObject *PyFunction::call_back(Args... args)
    {
        PyObject *pArgsPredict = PyTuple_Pack(sizeof...(args), args...);
        PyObject *ret = PyObject_CallObject(m_func, pArgsPredict);
        return ret;
    }

}

#endif