#include "daemon.h"

namespace lyserver
{
    static Logger::ptr g_loger = LY_LOG_NAME("system");
    daemon::daemon(const char *path, const char *name, const char *workdir) : path(path), name(name), workdir(workdir)
    {
    }
    daemon::~daemon()
    {
    }
    void daemon::work()
    {
        son_pid = fork();
        const char *path_ = path.c_str();
        const char *name_ = name.c_str();
        const char *workdir_ = workdir.c_str();
        if (son_pid < 0)
        {
            LY_LOG_ERROR(g_loger) << "Failed to fork.";
            exit(EXIT_FAILURE);
        }
        // 父进程进入
        if (son_pid > 0)
        {
            exit(EXIT_SUCCESS);
        }
        else if (son_pid == 0)
        {
            // 创建新会话并设置子进程为领头进程
            if (setsid() < 0)
            {
                LY_LOG_ERROR(g_loger) << "Failed to setsid.";
                exit(EXIT_FAILURE);
            }
            if (chdir(workdir_) < 0)
            {
                LY_LOG_ERROR(g_loger) << "Failed to chdir.";
                exit(EXIT_FAILURE);
            }
            int fd = open("/dev/null", O_RDWR);
            if (fd == -1)
            {
                LY_LOG_ERROR(g_loger) << "Failed to open /dev/null.";
                exit(EXIT_FAILURE);
            }

            dup2(fd, STDIN_FILENO);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            if (execl(path_, name_, NULL) == -1)
            {
                LY_LOG_ERROR(g_loger) << "Failed to execl";
            }
            printf("execl failed with result\n");
        }
    }
    void daemon::init(const char *path, const char *name, const char *workdir)
    {
        this->path = std::string(path);
        this->name = std::string(name);
        this->workdir = std::string(workdir);
    }
}
