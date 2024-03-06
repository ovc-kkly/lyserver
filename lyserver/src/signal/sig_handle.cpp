#include "sig_handle.h"

namespace lyserver
{
    std::unordered_map<int, std::pair<SigHandle::sigactionStruct *, SigHandle::sigactionStruct *>> SigHandle::sigHandlers;
    sigset_t SigHandle::myset;

    Signal g_received = Signal::SIGNOT_;//定义全局变量

    void handleSignal(int signum)
    {
        g_received = static_cast<Signal>(signum);
        switch (g_received)
        {
        case Signal::SIGNOT_:
            break;
        case Signal::SIGHUP_:
            // Handle SIGHUP
            break;
        case Signal::SIGINT_:
            handle_SIGINT((int)g_received);
            break;
        // ... 处理其他信号 ...
        default:
            // Handle unknown signal
            std::cout << "其他信号"
                      << "\n";
            break;
        }
    }
    void handle_SIGINT(int signum)
    {
        std::cout << signum << "exit"
                  << "\n";

        std::exit(0);
    }

    SigHandle::SigHandle()
    {

        // sigHandlers.insert(std::pair(SIGINT, oldAction));
    }

    SigHandle::~SigHandle()
    {
        // 在对象销毁时恢复原始的信号处理程序
        RestoreOriginalHandler();
    }

    // void SigHandle::SignalHandler(int signum)
    // {

    // }
    std::pair<SigHandle::sigactionStruct *, SigHandle::sigactionStruct *> SigHandle::pack_sig_handler(int signal, sig_handler handler)
    {
        sigactionStruct newAction;
        newAction.sa_handler = handler;
        newAction.sa_flags = 0;
        sigemptyset(&newAction.sa_mask);

        sigactionStruct oldAction;
        // 设置新的信号处理程序
        if (sigaction(signal, &newAction, &oldAction) == -1)
        {
            perror("sigaction");
            return std::make_pair(nullptr, nullptr);
        }

        return std::make_pair(&newAction, &oldAction);
    }
    void SigHandle::SetSignalHandler(int signal, sig_handler handler)
    {
        std::pair<SigHandle::sigactionStruct *, SigHandle::sigactionStruct *> handlers = pack_sig_handler(signal, handler);

        sigHandlers.insert(std::pair(signal, handlers));
    }

    void SigHandle::RestoreOriginalHandler()
    {
        // 恢复原始的信号处理程序
        for (auto it = sigHandlers.begin(); it != sigHandlers.end(); it++)
        {
            sigaction(it->first, it->second.second, nullptr);
        }
    }
}