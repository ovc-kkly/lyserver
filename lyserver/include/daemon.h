#ifndef DAEMON_H
#define DAEMON_H

#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include "log.h"
namespace lyserver
{
    class daemon : public std::enable_shared_from_this<daemon>
    {
    public:
        typedef std::shared_ptr<daemon> ptr;
        daemon(const char* path="", const char *name="", const char *workdir = "/");
        ~daemon();
        void init(const char* path, const char *name, const char *workdir = "/");

        void work();
    private:
        pid_t son_pid;

        std::string path;
        std::string name;
        std::string workdir;
    };
}

#endif