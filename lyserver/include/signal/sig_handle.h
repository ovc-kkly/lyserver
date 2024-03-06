#ifndef SIG_HANDLE_H
#define SIG_HANDLE_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <functional>
namespace lyserver
{
#define SIGNAL_NAME(XX)          \
    XX(0, SIGNOT, SIGNOT)        \
    XX(1, SIGHUP, SIGHUP)        \
    XX(2, SIGINT, SIGINT)        \
    XX(3, SIGQUIT, SIGQUIT)      \
    XX(4, SIGILL, SIGILL)        \
    XX(5, SIGTRAP, SIGTRAP)      \
    XX(6, SIGABRT, SIGABRT)      \
    XX(7, SIGFPE, SIGFPE)        \
    XX(8, SIGKILL, SIGKILL)      \
    XX(9, SIGSEGV, SIGSEGV)      \
    XX(10, SIGPIPE, SIGPIPE)     \
    XX(11, SIGALRM, SIGALRM)     \
    XX(12, SIGTERM, SIGTERM)     \
    XX(13, SIGUSR1, SIGUSR1)     \
    XX(14, SIGUSR2, SIGUSR2)     \
    XX(15, SIGCHLD, SIGCHLD)     \
    XX(16, SIGCONT, SIGCONT)     \
    XX(17, SIGSTOP, SIGSTOP)     \
    XX(18, SIGTSTP, SIGTSTP)     \
    XX(19, SIGTTIN, SIGTTIN)     \
    XX(20, SIGTTOU, SIGTTOU)     \
    XX(21, SIGURG, SIGURG)       \
    XX(22, SIGXCPU, SIGXCPU)     \
    XX(23, SIGXFSZ, SIGXFSZ)     \
    XX(24, SIGVTALRM, SIGVTALRM) \
    XX(25, SIGPROF, SIGPROF)     \
    XX(26, SIGWINCH, SIGWINCH)   \
    XX(27, SIGIO, SIGIO)         \
    XX(28, SIGPWR, SIGPWR)       \
    XX(29, SIGSYS, SIGSYS)

    // 信号枚举
    enum class Signal
    {
#define XX(num, name, string) name##_ = num,
        SIGNAL_NAME(XX)
#undef XX
            SIGINVALID_
    };
    extern Signal g_received;//进行声明

    // 信号处理函数
    void handleSignal(int signum);
    void handle_SIGINT(int signum);

    typedef void (*sig_handler)(int);
    // using sig_handler = std::function<void(int)>; // 包装器
    class SigHandle
    {
    public:
        typedef std::shared_ptr<SigHandle> ptr;
        typedef struct sigaction sigactionStruct;

        SigHandle();
        ~SigHandle();

    private:
        // static void SignalHandler(int signum);
        std::pair<sigactionStruct *, sigactionStruct *> pack_sig_handler(int signal, sig_handler handler);
        static std::unordered_map<int, std::pair<sigactionStruct *, sigactionStruct *>> sigHandlers;
        // 1. 初始化信号集
        static sigset_t myset;

    public:
        // 设置信号处理程序
        void SetSignalHandler(int signal, sig_handler handler);

        // 恢复原始的信号处理程序
        void RestoreOriginalHandler();
    };

}
#endif