#include "r_s_Msg.h"

namespace lyserver
{
    r_s_Msg::r_s_Msg(/* args */)
    {
    }

    r_s_Msg::~r_s_Msg()
    {
    }
    // 发送指定的字节数
    int r_s_Msg::writen(int fd, const char *msg, int size)
    {
        const char *buf = msg;
        int count = size;
        while (count > 0)
        {
            int len = send(fd, buf, count, 0);
            if (len == -1)
            {
                // errif(len == -1, "writen error");
                return -1;
            }
            else if (len == 0)
            {
                continue;
            }
            buf += len;
            count -= len;
        }
        return size;
    } /*
     |________________________
     |数据头   |   数据        |
     |________|_______________|
     */
    int r_s_Msg::sendMsg(int cfd, const char *msg, int len)
    {
        if (cfd < 0 || msg == NULL || len <= 0)
        {
            printf("发送数据函数报错\n");
            return -1;
        }
        char *data = (char *)malloc(len + 4); // 分配大小为len+4的内存
        int biglen = htonl(len);
        memcpy(data, &biglen, 4);
        memcpy(data + 4, msg, len);

        int ret = writen(cfd, data, len + 4);
        free(data);
        return ret;
    }
    // 接收指定字节个数的字符串
    int r_s_Msg::readen(int fd, char *buf, int size)
    {
        char *pt = buf;
        int count = size;
        while (count > 0)
        {
            int len = recv(fd, pt, count, 0);
            if (len == -1)
            {
                // printf("recv操作失败");
                return -1;
            }
            else if (len == 0)
            {
                printf("发送端断开了连接\n");
                return size - count;
            }
            pt += len; // 读一次指针后移len个位置
            count -= len;
        }
        return size;
    }
    int r_s_Msg::readMsg(int cfd, char **msg)
    {
        // 接收数据
        // 1. 读数据头
        // char len1[4];
        int len;
        int ret4 = readen(cfd, (char *)&len, 4);
        // printf("数据:%s/n", len1);
        // len = atoi(len1);
        if (ret4 == -1)
        {
            return ret4; // 表示没数据可读了或者有错误
        }
        else if (ret4 != 4) // 接收到一些，客户端断开了
        {
            return 0;
        }
        else if (ret4 == 5)
        {
        }
        len = ntohl(len); // 网络字节序转化成主机字节序
        // printf("接收的数据块大小: %d\n", len);

        // 根据读出的长度分配内存，+1 -> 这个字节存储\0
        char *buf = (char *)malloc(len + 1);
        int ret = readen(cfd, buf, len);
        if (ret != len) // 接收到一些，客户端断开了
        {
            printf("接收数据失败...接收的数据不完整\n");
            free(buf);
            return 0;
        }
        buf[len] = '\0';
        *msg = buf;

        return ret;
    }
}
