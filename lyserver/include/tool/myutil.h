#ifndef MYUTIL_H
#define MYUTIL_H

#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/signal.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <iomanip>
#include <fstream>

#include <execinfo.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <cxxabi.h>
#include "ocr_car.h"
#include "ThreadPool.h"

namespace lyserver
{
    // static thread_local std::string t_thread_name = "UNKNOW";
    using CHECK_CB = std::function<int()>; // 包装器
    void errif(bool, const char *);
    /**
     * @brief 获取当前时间的毫秒
     */
    uint64_t GetCurrentMS();

    /**
     * @brief 获取当前时间的微秒
     */
    uint64_t GetCurrentUS();
    /**
     * @brief 获取当前的调用栈
     * @param[out] bt 保存调用栈
     * @param[in] size 最多返回层数
     * @param[in] skip 跳过栈顶的层数
     */

    // pid_t GetThreadId();

    // uint32_t GetFiberId();
    // const std::string &GetName();
    void Backtrace(std::vector<std::string> &bt, int size = 64, int skip = 1);

    /**
     * @brief 获取当前栈信息的字符串
     * @param[in] size 栈的最大层数
     * @param[in] skip 跳过栈顶的层数
     * @param[in] prefix 栈信息前输出的内容
     */
    std::string BacktraceToString(int size = 64, int skip = 2, const std::string &prefix = "");
    std::string Time2Str(time_t ts = time(0), const std::string &format = "%Y-%m-%d %H:%M:%S");
    time_t Str2Time(const char *str, const char *format = "%Y-%m-%d %H:%M:%S");

    class FSUtil
    {
    public:
        static void ListAllFile(std::vector<std::string> &files, const std::string &path, const std::string &subfix);
        static bool Mkdir(const std::string &dirname);
        static bool IsRunningPidfile(const std::string &pidfile);
        static bool Rm(const std::string &path);
        static bool Mv(const std::string &from, const std::string &to);
        static bool Realpath(const std::string &path, std::string &rpath);
        static bool Symlink(const std::string &frm, const std::string &to);
        static bool Unlink(const std::string &filename, bool exist = false);
        static std::string Dirname(const std::string &filename);
        static std::string Basename(const std::string &filename);
        static bool OpenForRead(std::ifstream &ifs, const std::string &filename, std::ios_base::openmode mode);
        static bool OpenForWrite(std::ofstream &ofs, const std::string &filename, std::ios_base::openmode mode);
    };

    class StringUtil
    {
    public:
        static std::string Format(const char *fmt, ...);
        static std::string Formatv(const char *fmt, va_list ap);

        static std::string UrlEncode(const std::string &str, bool space_as_plus = true);
        static std::string UrlDecode(const std::string &str, bool space_as_plus = true);

        static std::string Trim(const std::string &str, const std::string &delimit = " \t\r\n");
        static std::string TrimLeft(const std::string &str, const std::string &delimit = " \t\r\n");
        static std::string TrimRight(const std::string &str, const std::string &delimit = " \t\r\n");

        static std::string WStringToString(const std::wstring &ws);
        static std::wstring StringToWString(const std::string &s);
    };
    template <class T>
    const char *TypeToName()
    {
        static const char *s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
        return s_name;
    }

}

#endif