#pragma once

#include <sys/socket.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>
#include "log.h"
// #include "myutil.h"
namespace lyserver
{
    class r_s_Msg
    {
    private:
    public:
        r_s_Msg(/* args */);
        ~r_s_Msg();

        static int writen(int fd, const char *msg, int size);
        static int sendMsg(int cfd, const char *msg, int len);
        static int readen(int fd, char *buf, int size);
        static int readMsg(int cfd, char **msg);
    };
}
