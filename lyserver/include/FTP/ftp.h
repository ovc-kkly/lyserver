#ifndef FTP_H
#define FTP_H

#include <memory>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <sys/stat.h>
#include <string.h>
#include <variant>
#include <json/json.h>
#include "r_s_Msg.h"
#include "ftp_cmd.h"
#include "My_events.h"
#include "sys/epoll.h"
#include "fdmanager.h"
using namespace Json;
using namespace std;

namespace lyserver
{
#define PACKET_SIZE (1036 - sizeof(int) * 3)
#define BUFF_SIZE 1024 * 1024

    /**
     * @brief 文件信息，包括文件大小，文件名
     *
     */
    struct FileInfo // 文件信息
    {
        int fileSize;
        char fileName[256];
        FileInfo()
        {
            fileSize = 0;
            memset(fileName, 0, sizeof(fileName));
        }
        FileInfo(const FileInfo &v)
        {
            this->fileSize = v.fileSize;
            strcpy(this->fileName, v.fileName);
        }
        FileInfo &operator=(const FileInfo &v)
        {
            this->fileSize = v.fileSize;
            strcpy(this->fileName, v.fileName);
            return *this;
        }
    };
    /**
     * @brief 文件包，包括
     *
     */
    struct packet // 文件包
    {
        int nStart;
        char filename_con[256];
        packet()
        {
            nStart = 0;
            memset(filename_con, 0, 256);
        }
        packet(const packet &v)
        {
            this->nStart = v.nStart;
            strcpy(this->filename_con, v.filename_con);
        }
        packet &operator=(const packet &v)
        {
            this->nStart = v.nStart;
            strcpy(this->filename_con, v.filename_con);
            return *this;
        }
    };
    /**
     * @brief 文件目录名称
     *
     */
    struct File_info_ // 目录
    {
        char name[100];
        int64_t size;
        bool isDir;
        File_info_()
        {
            size = 0;
            isDir = false;
            memset(name, 0, sizeof(name));
        }
        File_info_(const File_info_ &v)
        {
            this->size = v.size;
            this->isDir = v.isDir;
            strcpy(this->name, v.name);
        }
        File_info_ &operator=(const File_info_ &v)
        {
            this->size = v.size;
            this->isDir = v.isDir;
            strcpy(this->name, v.name);
            return *this;
        }
    };
    /**
     * @brief 目录包，包括文件目录，目录数量
     *
     */
    struct Dir_ // 目录包
    {
        int file_count;
        File_info_ file_list[20];
        Dir_()
        {
            file_count = 0;
            for (size_t i = 0; i < sizeof(file_list) / sizeof(file_list[0]); i++)
            {
                file_list[i] = File_info_();
            }
        }
        Dir_(const Dir_ &v)
        {
            this->file_count = v.file_count;
            for (size_t i = 0; i < sizeof(file_list) / sizeof(file_list[0]); i++)
            {
                this->file_list[i] = v.file_list[i];
            }
        }
        Dir_ &operator=(const Dir_ &v)
        {
            this->file_count = v.file_count;
            for (size_t i = 0; i < sizeof(file_list) / sizeof(file_list[0]); i++)
            {
                this->file_list[i] = v.file_list[i];
            }
            return *this;
        }
    };
    union MyUnion
    {
        FileInfo fileinfo;
        packet packet_;
        MyUnion()
        {
            fileinfo = FileInfo();
            packet_ = packet();
        }
        MyUnion(const MyUnion &v)
        {
            this->fileinfo = v.fileinfo;
            this->packet_ = v.packet_;
        }
        MyUnion &operator=(const MyUnion &v)
        {
            this->fileinfo = v.fileinfo;
            this->packet_ = v.packet_;
            return *this;
        }
    };

    struct BaseMsg
    {
        FTPRequestCommand ftp_request_cmd;
        FTPResponseCommand ftp_response_cmd;
        short port;

        BaseMsg()
        {
            ftp_request_cmd = FTPRequestCommand::INIT;
            ftp_response_cmd = FTPResponseCommand::INIT;
            port = 20;
        }
        // 拷贝构造函数
        BaseMsg(const BaseMsg &other)
        {
            ftp_request_cmd = other.ftp_request_cmd;
            port = other.port;
            ftp_response_cmd = other.ftp_response_cmd;
        }

        // 赋值操作符重载
        BaseMsg &operator=(const BaseMsg &other)
        {
            if (this != &other) // 避免自我赋值
            {
                ftp_request_cmd = other.ftp_request_cmd;
                port = other.port;
                ftp_response_cmd = other.ftp_response_cmd;
            }
            return *this;
        }
    };
    struct UseInfo : public BaseMsg
    {
        char Usename[50];
        char Password[50];
        UseInfo() : BaseMsg()
        {
            memset(Usename, 0, sizeof(Usename));
            memset(Password, 0, sizeof(Password));
        }
        // 拷贝构造函数
        UseInfo(const UseInfo &other) : BaseMsg(other)
        {

            // 复制 Usename 和 Password
            strncpy(Usename, other.Usename, sizeof(Usename));
            strncpy(Password, other.Password, sizeof(Password));
        }

        // 赋值操作符重载
        UseInfo &operator=(const UseInfo &other)
        {
            if (this != &other) // 避免自我赋值
            {
                // 赋值 BaseMsg 成员
                BaseMsg::operator=(other);

                // 赋值 Usename 和 Password
                strncpy(Usename, other.Usename, sizeof(Usename));
                strncpy(Password, other.Password, sizeof(Password));
            }
            return *this;
        }
    };
    struct FileInformation : public BaseMsg
    {
        MyUnion myuion;
        std::string file2end;
        std::string delefilename;
        FileInformation() : BaseMsg()
        {
            myuion = MyUnion();
            file2end = "";
            delefilename = "";
        }
        // 拷贝构造函数
        FileInformation(const FileInformation &other) : BaseMsg(other)
        {
            myuion = other.myuion;
            file2end = other.file2end;
            delefilename = other.delefilename;
        }

        // 赋值操作符重载
        FileInformation &operator=(const FileInformation &other)
        {
            if (this != &other) // 避免自我赋值
            {
                BaseMsg::operator=(other);
                myuion = other.myuion;
                file2end = other.file2end;
                delefilename = other.delefilename;
            }
            return *this;
        }
    };
    struct ContentInfo : public BaseMsg
    {
        int file_count;
        File_info_ file_list[30];
        char content_new[50];
        int type_up_or_down;
        std::string current_absolute_path;
        ContentInfo() : BaseMsg(), file_count(0), type_up_or_down(0), current_absolute_path("")
        {

            for (size_t i = 0; i < sizeof(file_list) / sizeof(file_list[0]); i++)
            {
                file_list[i] = File_info_();
            }
            memset(content_new, 0, 50);
        }
        // 拷贝构造函数
        ContentInfo(const ContentInfo &other) : BaseMsg(other)
        {
            file_count = other.file_count;
            type_up_or_down = other.type_up_or_down;
            current_absolute_path = other.current_absolute_path;
            strcpy(content_new, other.content_new);

            for (size_t i = 0; i < sizeof(file_list) / sizeof(file_list[0]); i++)
            {
                file_list[i] = other.file_list[i];
            }
        }

        // 赋值操作符重载
        ContentInfo &operator=(const ContentInfo &other)
        {
            if (this != &other) // 避免自我赋值
            {
                BaseMsg::operator=(other);
                file_count = other.file_count;
                type_up_or_down = other.type_up_or_down;
                current_absolute_path = other.current_absolute_path;
                strcpy(content_new, other.content_new);

                for (size_t i = 0; i < sizeof(file_list) / sizeof(file_list[0]); i++)
                {
                    file_list[i] = other.file_list[i];
                }
            }
            return *this;
        }
    };

    struct download_server2client_info
    {
        char fullpath[256];
        char *filebuf;
        uint64_t FileSize; // 文件大小
        FILE *file_fd;
    };
    struct download_client2server_info
    {
        uint64_t filesize;
        uint64_t filerecv;
        std::string file2end;
        char filename[256];
        char *filebuf;
        FILE *file;
    };
    class FTP
    {
    public:
        typedef std::shared_ptr<FTP> ptr;
        FTP()
        {
            strcpy(sys_useinfo.Usename, "ly");
            strcpy(sys_useinfo.Password, "ly");
        }
        /**
         * @brief 解析客户端发送的请求
         *
         * @param fd 客户端的文件描述符
         * @param msg 消息
         * @param file_fd 文件fd
         * @param free_filefd 是否释放文件fd
         */
        static void parse_(int fd, std::variant<FileInformation, BaseMsg, ContentInfo, UseInfo> &msg, Socket::ptr file_sock, bool *free_filefd);
        /**
         * @brief json转为结构体
         *
         * @param str
         * @return std::variant<FileInformation, BaseMsg, ContentInfo, UseInfo>
         */
        static std::variant<FileInformation, BaseMsg, ContentInfo, UseInfo> Json_to_struct(const Json::Value &obj);
        static std::string struct_to_Json(ContentInfo &msg);
        static std::string struct_to_Json(FileInformation &msg);
        static std::string struct_to_Json(BaseMsg &msg);
        static std::string struct_to_Json(UseInfo &msg);

        /**
         * @brief 用于检查客户端的登录信息是否对
         *
         * @param msg
         * @param fd
         */
        static void check(UseInfo &msg, int fd);
        /**
         * @brief 从路径中得到文件名
         *
         * @param path
         * @param filename_ 传出参数，
         */
        static void path_get_filename(char *path, const char **filename_);
        /**
         * @brief 发送目录
         *
         * @param dirName_msg
         * @param cfd
         */
        static void server_2_client_sendDir(ContentInfo *dirName_msg, int cfd);
        /**
         * @brief 打开文件fd,准备读取文件
         *
         * @param fif
         * @param cfd
         */
        static void client_2_server_ready_read(FileInformation *fif, int cfd);
        /**
         * @brief 接收客户端的文件
         *
         * @param buf
         * @param iRecv
         */
        static void client_2_server_recvFile(const char *buf, int64_t iRecv);
        static int filterHidden(const struct dirent *entry);
        // static void continue_send_file(FileInformation* con_read_file);
        /**
         * @brief 准备文件
         *
         * @param fd
         * @param pMsg
         * @return true
         * @return false
         */
        static bool server_2_client_ready_file(int fd, FileInformation *pMsg);
        /**
         * @brief 发送文件
         *
         * @param fd
         * @return true
         * @return false
         */
        static bool server_2_client_sendFile(int fd);

        static bool delefile_fun(FileInformation *filedele);

    private:
        static int m_fd;
        // 用户名，密码
        static UseInfo sys_useinfo;

        static const char *directory;
        static std::string dir_temp;
        // 下面是传输给客户端文件需要的变量
        // static char fullpath[256];
        // static char *filebuf;
        // static uint64_t FileSize; // 文件大小
        static download_server2client_info ds2c;
        // 下面是客户端上传文件给服务器需要的变量
        static download_client2server_info dc2s;
        // static uint64_t up_filesize;
        // static uint64_t up_filerecv;
        // static char up_filename[256];
        // static char *up_filebuf;
    };
}
#endif