#include "ftp.h"
#include <fcntl.h>
namespace lyserver
{
    static lyserver::Logger::ptr g_logger = LY_LOG_NAME("system");
    const char *FTP::directory = "/home/centos/mulu/file_";
    string FTP::dir_temp = "/home/ly";

    download_server2client_info FTP::ds2c = {"", nullptr, 0, nullptr};
    download_client2server_info FTP::dc2s = {0, 0, "", "", nullptr, nullptr};
    int FTP::m_fd = 0;
    // FILE *FTP::file = nullptr;
    UseInfo FTP::sys_useinfo; // 初始化静态数据成员

    /*解析客户端发送的请求*/
    void FTP::parse_(int fd, variant<FileInformation, BaseMsg, ContentInfo, UseInfo> &msg, Socket::ptr file_sock, bool *free_filefd)
    {
        *free_filefd = false;
        if (msg.index() == 0) // 表示数据类型是FileInformation
        {
            // 处理 FileInformation 类型的消息
            FileInformation *msg1 = &std::get<0>(msg);
            // cout << std::get<0>(msg).ftp_request_cmd;
            if (msg1->ftp_request_cmd == FTPRequestCommand::RETR)
            {
                FTP::server_2_client_ready_file(fd, msg1); // 发送前的准备文件
            }
            else if (msg1->ftp_request_cmd == FTPRequestCommand::STOR)
            {
                FTP::client_2_server_ready_read(msg1, fd);
                FTP::m_fd = fd;
            }
            else if (msg1->ftp_request_cmd == FTPRequestCommand::STOR_ING)
            {
                // printf("开始进入178\n");
                // FTP::read_up_file(msg1, fd);
            }
            else if (msg1->ftp_request_cmd == FTPRequestCommand::RETRCON)
            {

            }
            else if (msg1->ftp_request_cmd == FTPRequestCommand::DELE)
            {
                delefile_fun(msg1); // 删除文件
            }
            else
            {
                cout << "未知类型-";
            }
        }
        else if (msg.index() == 1) // 表示数据类型是BaseMsg
        {
            // 处理 BaseMsg 类型的消息
            // FTP::parse_<BaseMsg>(myev->m_fd, &std::get<1>(msg));
            BaseMsg msg1 = std::get<1>(msg);
            if (msg1.ftp_request_cmd == FTPRequestCommand::ALLO)
            {
                if (file_sock.use_count() > 0)
                {
                    server_2_client_sendFile(file_sock->getSocket());
                }
            }
            else if (msg1.ftp_request_cmd == FTPRequestCommand::MSG_SUCCESSED)
            {
                LY_LOG_INFO(g_logger) << "客户端接收完毕！";
                *free_filefd = true;
            }
            else if (msg1.ftp_request_cmd == FTPRequestCommand::ABOR)
            {
                *free_filefd = true;
            }
        }
        else if (msg.index() == 2) // 表示数据类型是ContentInfo
        {
            // 处理 ContentInfo 类型的消息
            ContentInfo msg1 = std::get<2>(msg);
            if (msg1.ftp_request_cmd == FTPRequestCommand::LIST)
            {
                server_2_client_sendDir(&msg1, fd);
            }
        }
        else if (msg.index() == 3) // 表示数据类型是UseInfo
        {
            UseInfo msg1 = std::get<3>(msg);
            if (msg1.ftp_request_cmd == FTPRequestCommand::USER)
            {
                check(msg1, fd);
            }
        }
    }
    /*json字符串转换为结构体*/
    std::variant<FileInformation, BaseMsg, ContentInfo, UseInfo> FTP::Json_to_struct(const Json::Value &obj)
    {
        std::string info = obj["FTP_REQUEST"].asString();

        if (info == "RETR")
        {
            info = obj["filename"].asString(); // 获取了文件名

            FileInformation msg;
            msg.ftp_request_cmd = FTPRequestCommand::RETR;
            strcpy(msg.myuion.fileinfo.fileName, info.c_str());
            return msg;
        }
        else if (info == "RETRCON")
        {
            FileInformation msg;
            msg.ftp_request_cmd = FTPRequestCommand::RETRCON;
            msg.myuion.packet_.nStart = obj["nStart"].asInt();
            string filename_con;
            filename_con = obj["filename_continue"].asString();
            strcpy(msg.myuion.packet_.filename_con, filename_con.c_str());
            return msg;
        }
        else if (info == "ALLO") // 这个表示客户端准备好了内存
        {
            BaseMsg msg;
            msg.ftp_request_cmd = FTPRequestCommand::ALLO;
            return msg;
        }
        else if (info == "LIST")
        {
            string content_new;
            int type = obj["type_up_or_down"].asInt();
            content_new = obj["content_new"].asString();
            printf("目录:%s\n", content_new.c_str());
            ContentInfo msg;
            msg.ftp_request_cmd = FTPRequestCommand::LIST;
            msg.type_up_or_down = type;
            strcpy(msg.content_new, content_new.c_str());
            return msg;
        }
        else if (info == "STOR")
        {
            string filename, file2end;
            int filesize;
            filename = obj["filename"].asString(); // 获取了文件名
            filesize = obj["filesize"].asInt();
            file2end = obj["file2end"].asString(); // 文件目的地

            FileInformation msg;
            msg.ftp_request_cmd = FTPRequestCommand::STOR;
            msg.myuion.fileinfo.fileSize = filesize;
            strcpy(msg.myuion.fileinfo.fileName, filename.c_str());
            msg.file2end = file2end;
            return msg;
        }
        else if (info == "STOR_ING")
        {
            // int nStart, nsize;
            // string packet_buf;
            // nStart = obj["nStart"].asInt();
            // nsize = obj["nsize"].asInt();
            // packet_buf = obj["packet_buf"].asString();

            // FileInformation msg;
            // msg.ftp_request_cmd = FTPRequestCommand::STOR_ING;
            // msg.myuion.packet_.nStart = nStart;
            // msg.myuion.packet_.nsize = nsize;
            // strcpy(msg.myuion.packet_.buf, packet_buf.c_str());
            // if(msg.ftp_request_cmd == FTPRequestCommand::STOR_ING){
            //     cout << "类型正确" <<endl;
            // }else{
            //     cout << "错"<<endl;
            // }
            // return msg;
        }
        else if (info == "USER")
        {
            string user, passward;
            user = obj["user"].asString();
            passward = obj["passward"].asString();

            UseInfo usermsg;
            usermsg.ftp_request_cmd = FTPRequestCommand::USER;
            strcpy(usermsg.Usename, user.c_str());
            strcpy(usermsg.Password, user.c_str());
            return usermsg;
        }
        else if (info == "MSG_SUCCESSED")
        {
            BaseMsg msg;
            msg.ftp_request_cmd = FTPRequestCommand::MSG_SUCCESSED;
            return msg;
        }
        else if (info == "ABOR")
        {
            BaseMsg msg;
            msg.ftp_request_cmd = FTPRequestCommand::ABOR;
            return msg;
        }
        else if (info == "DELE")
        {
            FileInformation msg;
            msg.ftp_request_cmd = FTPRequestCommand::DELE;
            msg.delefilename = obj["delefilename"].asString();
            return msg;
        }
        return BaseMsg();
    }

    string FTP::struct_to_Json(ContentInfo &msg)
    {
        Value obj;
        if (msg.ftp_response_cmd == FTPResponseCommand::Code151)
        {
            obj["ftp_response_cmd"] = "Code151";
            obj["file_count"] = msg.file_count;
            obj["current_absolute_path"] = Value(msg.current_absolute_path);
            Value array, subobj;
            for (int i = 0; i < msg.file_count; i++)
            {
                string str(msg.file_list[i].name);
                subobj["file_list_name"] = Value(str);
                subobj["file_list_size"] = msg.file_list[i].size;
                subobj["file_list_isDir"] = msg.file_list[i].isDir;
                array.append(subobj); // Json数组里添加对象
            }
            obj["file_list"] = array;
        }
        return obj.toStyledString();
    }
    string FTP::struct_to_Json(FileInformation &msg)
    {
        Value obj;
        if (msg.ftp_response_cmd == FTPResponseCommand::Code150)
        {
            obj["ftp_response_cmd"] = "Code150";
            obj["filesize"] = msg.myuion.fileinfo.fileSize;
            string str(msg.myuion.fileinfo.fileName);
            obj["filename"] = str;
        }
        else if (msg.ftp_response_cmd == FTPResponseCommand::Code125)
        {
            // obj["ftp_response_cmd"] = "Code125";
            // // obj["nsize"] = msg.myuion.packet_.nsize;
            // // obj["nStart"] = msg.myuion.packet_.nStart;
            // string str(msg.myuion.packet_.buf);
            // obj["buf"] = str;
        }
        return obj.toStyledString();
    }
    string FTP::struct_to_Json(BaseMsg &msg)
    {
        Value obj;
        if (msg.ftp_response_cmd == FTPResponseCommand::Code200)
        {
            obj["ftp_response_cmd"] = "Code200";
        }
        else if (msg.ftp_response_cmd == FTPResponseCommand::Code220)
        {
            obj["ftp_response_cmd"] = "Code220";
            obj["port"] = msg.port;
        }
        return obj.toStyledString();
    }
    string FTP::struct_to_Json(UseInfo &msg)
    {
        Value obj;
        if (msg.ftp_response_cmd == FTPResponseCommand::Code230)
        {
            obj["ftp_response_cmd"] = "Code230";
        }
        else if (msg.ftp_response_cmd == FTPResponseCommand::Code530)
        {
            obj["ftp_response_cmd"] = "Code530";
        }
        return obj.toStyledString();
    }
    void FTP::check(UseInfo &msg, int fd)
    {
        if (strcmp(msg.Usename, sys_useinfo.Usename) == 0)
        {
            if (strcmp(msg.Password, sys_useinfo.Password) == 0)
            {
                UseInfo msg;
                msg.ftp_response_cmd = FTPResponseCommand::Code230;
                string user_p_response = FTP::struct_to_Json(msg);
                int ret = r_s_Msg::sendMsg(fd, user_p_response.c_str(), strlen(user_p_response.c_str()));
                if (ret > 0)
                {
                    LY_LOG_ERROR(g_logger) << "成功登录";
                }
            }
            else
            {
                UseInfo msg;
                msg.ftp_response_cmd = FTPResponseCommand::Code530;
                string user_p_response = FTP::struct_to_Json(msg);
                int ret = r_s_Msg::sendMsg(fd, user_p_response.c_str(), strlen(user_p_response.c_str()));
                if (ret > 0)
                {
                    LY_LOG_ERROR(g_logger) << "密码错误";
                }
            }
        }
        else
        {
            UseInfo msg;
            msg.ftp_response_cmd = FTPResponseCommand::Code530;
            string user_p_response = FTP::struct_to_Json(msg);
            int ret = r_s_Msg::sendMsg(fd, user_p_response.c_str(), strlen(user_p_response.c_str()));
            if (ret > 0)
            {
                LY_LOG_ERROR(g_logger) << "用户名错误";
            }
        }
    }
    void FTP::path_get_filename(char *path, const char **filename_)
    {
        const char *file_name = strrchr(path, '/'); // 如果str中存在字符'/',返回最后出现'/'的位置的指针，否则返回NULL

        if (file_name != NULL)
        {
            file_name++; // 移动指针到文件名的开头
        }
        else
        {
            file_name = path; // 没有找到 '/'，文件名就是路径本身
        }
        *filename_ = file_name;
    }
    /*准备文件*/
    bool FTP::server_2_client_ready_file(int fd, FileInformation *pMsg)
    {
        const char *file_name = nullptr;
        // 找出文件名
        path_get_filename(pMsg->myuion.fileinfo.fileName, &file_name);

        snprintf(ds2c.fullpath, sizeof(ds2c.fullpath), "%s/%s", dir_temp.c_str(), file_name);
        LY_LOG_INFO(g_logger) << ds2c.fullpath;
        ds2c.file_fd = fopen(ds2c.fullpath, "rb");
        if (ds2c.file_fd == NULL)
        {
            LY_LOG_INFO(g_logger) << "找不到[" << file_name << "]文件";
            BaseMsg msg;
            msg.ftp_response_cmd = FTPResponseCommand::Code550;
            if (send(fd, (char *)&msg, sizeof(BaseMsg), 0) == -1)
            {
                printf("send faild\n");
            }
            return false;
        }

        fseek(ds2c.file_fd, 0, SEEK_END);
        // 获取文件的 当前指针位置 相对于 文件首地址 的 偏移字节数
        off64_t file_size = ftell(ds2c.file_fd);
        fseek(ds2c.file_fd, 0, SEEK_SET);
        FTP::ds2c.FileSize = file_size;
        FileInformation msg2;
        msg2.ftp_response_cmd = FTPResponseCommand::Code150;
        msg2.myuion.fileinfo.fileSize = file_size; // 添加文件大小
        msg2.port = 20;

        strcpy(msg2.myuion.fileinfo.fileName, file_name); // 拷贝文件名到 fileName 数组
        string msg_str = FTP::struct_to_Json(msg2);
        const char *tempstr = msg_str.c_str();
        // send(fd, msg_str.c_str(), strlen(msg_str), 0); // 发送了文件大小和文件名
        r_s_Msg::sendMsg(fd, tempstr, strlen(tempstr));
        LY_LOG_INFO(g_logger) << "发送了:" << tempstr;

        return false;
    }
    int FTP::filterHidden(const struct dirent *entry)
    {
        // 检查文件名是否以点开头，是的话排除
        if (entry->d_name[0] == '.')
        {
            return 0; // 返回0表示不包括
        }
        return 1; // 返回1表示包括
    }
    /*发送目录*/
    void FTP::server_2_client_sendDir(ContentInfo *dirName_msg, int cfd)
    {
        char *dirName = nullptr;
        // 分配足够的内存用于存储 content_new 的内容
        dirName = new char[strlen(dirName_msg->content_new) + 1];
        dirName[strlen(dirName_msg->content_new) + 1] = '\0';
        strcpy(dirName, dirName_msg->content_new);
        // 更新当前目录
        if (dirName_msg->type_up_or_down == 1)
        { // 表示进入目录
            dir_temp.append("/");
            dir_temp.append(string(dirName));
        }
        else if (dirName_msg->type_up_or_down == 2)
        { // 表示退到上级目录
            // 找到最后一个斜杠的位置
            size_t lastSlashPos = dir_temp.find_last_of('/');

            if (lastSlashPos != std::string::npos)
            {
                // 删除最后一个斜杠后面的内容
                dir_temp = dir_temp.substr(0, lastSlashPos); // +1 保留斜杠
            }
        }

        struct dirent **namelist;                                                // 它指向的是指针数组struct dirent* name[];
        int num = scandir(dir_temp.c_str(), &namelist, filterHidden, alphasort); // 获取指定路径下的文件目录
        ContentInfo file_dir;
        file_dir.ftp_response_cmd = FTPResponseCommand::Code151;
        file_dir.file_count = num;
        file_dir.current_absolute_path = dir_temp; // 绝对路径
        LY_LOG_INFO(g_logger) << "在路径:" << file_dir.current_absolute_path << "目录有:" << num;
        for (int i = 0; i < num; i++)
        {
            struct stat st;
            const char *name = namelist[i]->d_name;

            strcpy(file_dir.file_list[i].name, name);
            string strtemp = dir_temp + "/" + string(name);
            stat(strtemp.c_str(), &st);
            file_dir.file_list[i].size = st.st_size;
            if (S_ISDIR(st.st_mode))
            {
                file_dir.file_list[i].isDir = true;
            }
            else
            {
                file_dir.file_list[i].isDir = false;
            }
            free(namelist[i]);
        }
        string str_file_dir = FTP::struct_to_Json(file_dir);
        // LY_LOG_INFO(g_logger) <<"内容:"<<str_file_dir;
        int ret = r_s_Msg::sendMsg(cfd, str_file_dir.c_str(), strlen(str_file_dir.c_str()));
        if (ret == -1)
        {
            LY_LOG_ERROR(g_logger) << "发送错误";
        }
        else
        {
            LY_LOG_INFO(g_logger) << "本应该的字节："<< str_file_dir.size() <<"发送成功,大小字节:" << ret;
        }
        delete[] dirName;
        dirName = nullptr;
        free(namelist);
    }
    /*发送文件*/
    bool FTP::server_2_client_sendFile(int fd)
    {
        LY_LOG_ERROR(g_logger) << "开始发送....," << ds2c.fullpath;

        uint64_t sendSize = 0;
        ds2c.filebuf = (char *)malloc(sizeof(char) * (BUFF_SIZE)); // 申请2mb的缓存空间
        while (sendSize < ds2c.FileSize)
        {
            memset(ds2c.filebuf, 0, BUFF_SIZE);
            int64_t iread = fread(ds2c.filebuf, sizeof(char), BUFF_SIZE, ds2c.file_fd);

            if (ferror(ds2c.file_fd))
            {

                LY_LOG_ERROR(g_logger) << "Error reading file";
                break;
            }
            while (iread)
            {
                int64_t iSend = send(fd, ds2c.filebuf, iread, 0);

                // printf("iSend:%lu\n", iSend);
                if (iSend < 0)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        // 等待一段时间后重试
                        LY_LOG_WARN(g_logger) << "写缓冲区满了,稍等...";
                        usleep(10000000); // 1000ms

                        continue;
                    }
                    else if (errno == EPIPE)
                    {
                        LY_LOG_WARN(g_logger) << "对端已经关闭";
                        return false;
                    }
                    else
                    {
                        LY_LOG_ERROR(g_logger) << "send error: " << strerror(errno);
                        // fclose(fd);
                        usleep(10000000); // 1000ms
                        continue;
                    }
                }
                if (iSend > 0)
                {
                    sendSize += iSend;
                    LY_LOG_INFO(g_logger) << "filesize:" << ds2c.FileSize << "iSend:" << iSend << "sendSize:" << sendSize;
                    fseeko64(ds2c.file_fd, sendSize, SEEK_SET);
                    iread = 0;
                }
            }

            usleep(100000); // 100ms
        }

        fclose(ds2c.file_fd);
        LY_LOG_INFO(g_logger) << "Server ended successfully";
        return true;
    }
    /*申请内存*/
    void FTP::client_2_server_ready_read(FileInformation *fif, int cfd)
    {
        // 准备内存
        int filesize = fif->myuion.fileinfo.fileSize;
        dc2s.filesize = filesize;
        const char *temp = nullptr;
        path_get_filename(fif->myuion.fileinfo.fileName, &temp);
        strcpy(dc2s.filename, temp);
        dc2s.file2end = fif->file2end;

        std::string fullpath = dc2s.file2end.append("/").append(std::string(dc2s.filename));
        dc2s.file = fopen(fullpath.c_str(), "ab");

        BaseMsg filemsg;
        filemsg.port = 20;
        filemsg.ftp_response_cmd = FTPResponseCommand::Code220;
        // send(cfd, (char *)&filemsg, sizeof(filemsg), 0);
        string str_filemsg = FTP::struct_to_Json(filemsg);
        const char *str_const = str_filemsg.c_str();
        r_s_Msg::sendMsg(cfd, str_const, strlen(str_const));
        LY_LOG_INFO(g_logger) << "准备好了内存, 文件名:" << dc2s.filename << "大小:" << dc2s.filesize << "保存在:" << dc2s.file2end;
    }
    void FTP::client_2_server_recvFile(const char *buf, int64_t iRecv)
    {
        int64_t written = fwrite(buf, sizeof(char), iRecv, dc2s.file);
        if (written == iRecv)
        {
            dc2s.filerecv += iRecv;
        }
        else
        {
            LY_LOG_ERROR(g_logger) << "写入不完整";
        }
        LY_LOG_INFO(g_logger) << "文件总大小:" << dc2s.filesize << ", 每一次接收了:" << iRecv << ", 累计接收了：" << dc2s.filerecv;
        if (dc2s.filerecv >= dc2s.filesize)
        {
            dc2s.filerecv = 0;
            dc2s.filesize = 0;
            memset(dc2s.filename, 0, sizeof(dc2s.filename));
            // 写完释放缓存，关闭读指针
            fclose(dc2s.file);
            // 发送写入成功消息
            BaseMsg msg;
            msg.ftp_response_cmd = FTPResponseCommand::Code200;
            string str_msg = FTP::struct_to_Json(msg);
            if (r_s_Msg::sendMsg(m_fd, str_msg.c_str(), strlen(str_msg.c_str())) < 0)
            {
                LY_LOG_ERROR(g_logger) << "发送错误";
                return;
            }
            else
            {
                LY_LOG_INFO(g_logger) << "接收完成";
            }
        }
    }
    // 删除文件
    bool FTP::delefile_fun(FileInformation *filedele)
    {
        std::string delefilename_ = filedele->delefilename;
        // 使用 unlink 函数删除文件
        // 使用 sudo 和 rm 命令删除文件
        std::string command = "sudo rm -rf " + delefilename_;

        // 使用 system 函数执行命令
        int result = system(command.c_str());

        // 检查命令执行结果
        if (result == 0)
        {
            LY_LOG_INFO(g_logger) << "文件:"<< command<<"删除成功";
            return true;
        }
        else
        {
            LY_LOG_INFO(g_logger)  <<"文件:"<< command<< "删除失败";
            return false;
        }
        return false;
    }
}