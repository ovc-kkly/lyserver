#include "EventHandle.h"

namespace lyserver
{
    static lyserver::Logger::ptr g_logger = LY_LOG_NAME("system");
    string EventHandle::image = "";
    EventHandle *EventHandle::evHandle = nullptr;
    EventHandle::EventHandle(/* args */)
    {
        // std::cout << "引用计数:"<<Conn_Pool.use_count() << "地址:"<<&Conn_Pool<<'\n';
        user_ = new user_db_handle;
        length = 0;
        fdmagr = FdMgr_::GetInstance()->getFdmgr_ptr(); // fd存储结构
        ftp.reset(new FTP);
        rf.reset(new RF);
        pl.reset(new PersonaLization(std::shared_ptr<user_db_handle>(user_)));
    }

    EventHandle::~EventHandle()
    {
    }
    void EventHandle::register_Event_callback()
    {
        callback_map.insert(std::pair<EventType, Event_Callback>(EventType::ID_REQUEST, std::bind(&EventHandle::set_client_ID, this, std::placeholders::_1, std::placeholders::_2)));
        callback_map.insert(std::pair<EventType, Event_Callback>(EventType::LOGON_REQUEST, std::bind(&EventHandle::Log_on_CallBack, this, std::placeholders::_1, std::placeholders::_2)));
        callback_map.insert(std::pair<EventType, Event_Callback>(EventType::SIGNUP_REQUEST, std::bind(&EventHandle::Sign_up_CallBack, this, std::placeholders::_1, std::placeholders::_2)));
        callback_map.insert(std::pair<EventType, Event_Callback>(EventType::CLOSE_ACCOUNT_REQUEST, std::bind(&EventHandle::Close_account_CallBack, this, std::placeholders::_1, std::placeholders::_2)));
        callback_map.insert(std::pair<EventType, Event_Callback>(EventType::LOG_OUT_REQUEST, std::bind(&EventHandle::Log_out_CallBack, this, std::placeholders::_1, std::placeholders::_2)));
        callback_map.insert(std::pair<EventType, Event_Callback>(EventType::RECOGNIZE, std::bind(&EventHandle::recognize_CallBack, this, std::placeholders::_1, std::placeholders::_2)));
        callback_map.insert(std::pair<EventType, Event_Callback>(EventType::IMAGE_LENGTH, std::bind(&EventHandle::imageLength_CallBack, this, std::placeholders::_1, std::placeholders::_2)));
        callback_map.insert(std::pair<EventType, Event_Callback>(EventType::FTP_REQUEST, std::bind(&EventHandle::ftp_CallBack, this, std::placeholders::_1, std::placeholders::_2)));
        callback_map.insert(std::pair<EventType, Event_Callback>(EventType::DETECTION_DATA, std::bind(&EventHandle::detection_data_CallBack, this, std::placeholders::_1, std::placeholders::_2)));

        callback_map.insert(std::pair<EventType, Event_Callback>(EventType::RF_EXECUTE, std::bind(&EventHandle::RF_execute_CallBack, this, std::placeholders::_1, std::placeholders::_2)));
        callback_map.insert(std::pair<EventType, Event_Callback>(EventType::RF_DATASET_ONE, std::bind(&EventHandle::recv_dataset_one, this, std::placeholders::_1, std::placeholders::_2)));
        callback_map.insert(std::pair<EventType, Event_Callback>(EventType::RETURN_REQUEST, std::bind(&EventHandle::return_CallBack, this, std::placeholders::_1, std::placeholders::_2)));
    }
    int EventHandle::respond_client(EventType &ev, My_events *myev)
    {
        Json::Value response_info;
        response_info["RESPONSE"] = string(EventTypeToString(ev));
        string str = response_info.toStyledString();

        return r_s_Msg::sendMsg(GET_SOCK_FD(myev), str.c_str(), strlen(str.c_str()));
    }
    int EventHandle::respond_client_(EventType &ev, My_events* myev)
    {
        Json::Value response_info;
        response_info["RESPONSE"] = string(EventTypeToString(ev));
        string str = response_info.toStyledString();
        str += "\r\n";

        return myev->sockptr->send(str.c_str(), strlen(str.c_str()));
    }
    int EventHandle::return_CallBack(const Json::Value &obj, My_events* myev)
    {
        EventType ev = EventType::RETURN_REQUEST;
        int ret = respond_client_(ev, myev);
        return ret;
    }
    int EventHandle::ID_CallBack(const Json::Value &obj, My_events *myev)
    {
        return 0;
    }
    int EventHandle::Log_on_CallBack(const Json::Value &obj, My_events *myev)
    {
        int ret;
        EventType E;
        std::unique_ptr<user_db_handle, Deleter_resource> ptr(user_, Deleter_resource());
        int ret_ = ptr->isExistUser(obj["UserID"].asString());
        if (ret_ > 0)
        {
            if (ptr->passward_right_wrong(obj["password"].asString()))
            {
                // 密码对
                E = EventType::LOGON_RESPOND_PWRIGHT;
                ptr->change_user_log_on_state(obj["UserID"].asString(), true);
                notify_u(obj["UserID"].asString());
                ret = respond_client(E, myev);

                pl->decide_userid_modelpath(obj["UserID"].asString()); // 决策用户使用哪个模型
            }
            else
            {
                // 密码错误
                E = EventType::LOGON_RESPOND_PWERROR;
                ret = respond_client(E, myev);
            }
        }
        else if (ret_ == 0)
        {
            // 表示无该用户
            E = EventType::LOGON_RESPOND_NOUSER;
            ret = respond_client(E, myev);
        }
        else if (ret_ == -1)
        {
            // 查询失败
            E = EventType::QUERY_RESPOND_ERROR;
            LY_LOG_ERROR(g_logger) << "查询错误,sql语句有错或者连接池有问题";
            ret = respond_client(E, myev);
        }
        return ret;
    }
    int EventHandle::Sign_up_CallBack(const Json::Value &obj, My_events *myev)
    {
        int ret;
        EventType E;
        std::unique_ptr<user_db_handle, Deleter_resource> ptr(user_, Deleter_resource());
        int ret_ = ptr->isExistUser(obj["UserID"].asString());
        if (ret_ == 0)
        {
            if (ptr->insert_userID(obj["UserID"].asString(), obj["password"].asString()))
            {
                // 插入成功
                E = EventType::SIGNUP_RESPOND_SUCCESS;
                ret = respond_client(E, myev);

                std::string model_dir_path;

                pl->create_loacal_model_dir(obj["UserID"].asString(), model_dir_path); // 注册成功，创建模型文件夹

                pl->change_user_2_modeldir(obj["UserID"].asString(), model_dir_path); // 创建文件夹成功，就更新数据库当中的用户对应的模型文件夹路径dir
                pl->create_user_dataSet_table(obj["UserID"].asString());              // 创建空的数据集路径，即表名
            }
        }
        else if (ret_ > 0)
        {
            // 表示该用户已经存在
            E = EventType::SIGNUP_RESPOND_EXIST;
            ret = respond_client(E, myev);
        }
        else if (ret_ == -1)
        {
            // 查询失败
            E = EventType::QUERY_RESPOND_ERROR;
            LY_LOG_ERROR(g_logger) << "查询错误,sql语句有错或者连接池有问题";
            ret = respond_client(E, myev);
        }
        return ret;
    }
    int EventHandle::Transmit_CallBack(const Json::Value &obj, My_events *myev)
    {
        return 0;
    }
    int EventHandle::Close_account_CallBack(const Json::Value &obj, My_events *myev)
    {
        int ret;
        EventType E;
        std::unique_ptr<user_db_handle, Deleter_resource> ptr(user_, Deleter_resource());
        int ret_ = ptr->query_user_info(obj["UserID"].asString(), obj["password"].asString());
        if (ret_ == 1)
        {
            // 删除成功
            E = EventType::CLOSE_RESPOND_SUCCESS;
            ret = respond_client(E, myev);
            ptr->change_user_log_on_state(obj["UserID"].asString(), false);
            pl->clear(obj["UserID"].asString());
        }
        else if (ret_ == 0)
        {
            // 删除失败
            E = EventType::CLOSE_RESPOND_ERROR;
            ret = respond_client(E, myev);
        }
        else if (ret_ == -1)
        {
            // 查询失败或用户不存在
            E = EventType::QUERY_RESPOND_ERROR;
            ret = respond_client(E, myev);
        }
        else
        {
            // 密码错误
            E = EventType::CLOSE_RESPOND_PWERROR;
            ret = respond_client(E, myev);
        }
        return ret;
    }

    int EventHandle::set_client_ID(const Json::Value &obj, My_events *myev)
    {
        string ID = obj["ID"].asString();
        int st = fdmagr->whoID(myev, ID);
        if (st == 1)
        {
            LY_LOG_INFO(g_logger) << "[身份确认,ID:" << myev->ID.c_str() << "]";
            return -1;
        }
        else if (st == -1)
        {
            LY_LOG_ERROR(g_logger) << "[身份确认失败]";
            // 清楚现在的连接
            fdmagr->free_client_resources(myev, "已经释放已经连接的", false, fdmagr->get_epollmgr(myev));
            return -3;
        }
        else if (st == 0)
        {
            LY_LOG_ERROR(g_logger) << "[未知ID]";
            // 清楚现在的连接
            fdmagr->free_client_resources(myev, "已经释放已经连接的", false, fdmagr->get_epollmgr(myev));
            return -3; // 未知ID应该跳出循环
        }
        else if (st == -2)
        {
            LY_LOG_ERROR(g_logger) << "[已经存在该ID]";
            // 清除现在的连接
            fdmagr->free_client_resources(myev, "已经释放已经连接的", true, fdmagr->get_epollmgr(myev));
            return -3;
        }
        return -3;
    }
    int EventHandle::Log_out_CallBack(const Json::Value &obj, My_events *myev)
    {
        std::unique_ptr<user_db_handle, Deleter_resource> ptr(user_, Deleter_resource());
        ptr->change_user_log_on_state(obj["UserID"].asString(), false);
        notify_u("general");
        LY_LOG_INFO(g_logger) << "退出登录成功";
        return 0;
    }
    void EventHandle::notify_u(const string &userID)
    {
        string vc = "uportrait";
        int uportrait_fd = fdmagr->ID_2_fd(vc);
        if (uportrait_fd == -1)
        {
            LY_LOG_INFO(g_logger) << "uportrait未登录";
            return;
        }
        Json::Value uportrait_info;
        uportrait_info["NOTIFY"] = "USER_CHANGE";
        uportrait_info["current_userid"] = userID;
        string str = uportrait_info.toStyledString();

        r_s_Msg::sendMsg(uportrait_fd, str.c_str(), strlen(str.c_str()));
    }
    int EventHandle::imageLength_CallBack(const Json::Value &obj, My_events *myev)
    {
        int ret;
        EventType E;
        my_ocr::ocr_ptr ocrCar_ptr(new my_ocr);
        // EventHandle::ocr_sdk_ptr ptr(new aip::Ocr(this->app_id, this->api_key, this->secret_key));

        this->length = obj["length"].asInt();
        EventHandle::image.reserve(length);
        LY_LOG_INFO(g_logger) << "图片的大小：" << length;
        E = EventType::READY_READ;
        Json::Value response_info;
        response_info["RESPONSE"] = string(EventTypeToString(E));
        string str = response_info.toStyledString();
        ret = send(GET_SOCK_FD(myev), str.c_str(), strlen(str.c_str()), 0);
        std::cout << "发送了：" << ret << std::endl;
        LY_LOG_INFO(g_logger) << "发送了：" << ret;

        /*
        开始接收图片
        */
        int flags = Socket::sockfcntl(GET_SOCK_FD(myev), F_GETFL, 0);
        if (flags == -1)
        {
            LY_LOG_ERROR(g_logger) << "fcntl";
            return -1;
        }
        flags &= ~O_NONBLOCK; // 清除非阻塞标志
        if (Socket::sockfcntl(GET_SOCK_FD(myev), F_SETFL, flags) == -1)
        {
            LY_LOG_ERROR(g_logger) << "fcntl";
            return -1;
        }
        size_t count = 0;
        char buf[4098];
        while (1)
        {

            memset(buf, 0, sizeof(buf));
            int ret = recv(GET_SOCK_FD(myev), buf, sizeof(buf), 0);
            if (ret > 0)
            {
                count += ret;
                EventHandle::image.append(string(buf));
                LY_LOG_INFO(g_logger) << "接收到:" << ret << "累积：" << count << "总字节数：" << length << "image的数据:" << image.size();
                if (count == length)
                {
                    LY_LOG_INFO(g_logger) << "接收完毕";

                    if ((Socket::sockfcntl(GET_SOCK_FD(myev), F_SETFL, O_NONBLOCK)) < 0) // 将cfd设置为非阻塞
                    {
                        LY_LOG_ERROR(g_logger) << ": fcntl nonblocking failed," << strerror(errno);
                        break;
                    }
                    break;
                }
            }
            else if (ret == 0)
            {
                LY_LOG_ERROR(g_logger) << "断开了";
            }
            else if (ret < 0)
            {

                if (errno == EAGAIN)
                {
                    LY_LOG_WARN(g_logger) << "没数据了";

                    break;
                }
            }
        }
        dete(ocrCar_ptr, image, GET_SOCK_FD(myev));
        return ret;
    }
    void EventHandle::dete(my_ocr::ocr_ptr &ptr, string &image, const int &fd)
    {

        std::string img = ptr->myocr_base64_decode(image);
        ptr->Recognize(img);
        std::string result = ptr->getresult();
        std::cout << result << std::endl;
        send(fd, result.c_str(), strlen(result.c_str()), 0);
    }
    int EventHandle::recognize_CallBack(const Json::Value &obj, My_events *myev)
    {
        // My_events *myev = (My_events *)myev_;
        std::string img = obj["image"].asString();
        std::cout << img.size() << endl;

        EventHandle::image.append(obj["image"].asString());
        if (image.size() == length)
        {
            LY_LOG_INFO(g_logger) << "接收完毕";
        }
        return 1;
    }
    int EventHandle::ftp_CallBack(const Json::Value &obj, My_events *myev)
    {
        bool free_file_fd = false;
        std::variant msg = FTP::Json_to_struct(obj);
        FTP::parse_(GET_SOCK_FD(myev), msg, myev->m_file_event->sockptr, &free_file_fd);
        if (free_file_fd)
        {
            fdmagr->free_client_resources(myev->m_file_event, "传输端口释放了", false, fdmagr->get_epollmgr(myev));
        }
        return 1;
    }
    int EventHandle::detection_data_CallBack(const Json::Value &obj, My_events *myev)
    {
        LY_LOG_INFO(g_logger) << obj["ID"].asCString();
        return 1;
    }

    string EventHandle::get_info(const string &buff, const string &need)
    {
        Value obj;
        Reader r;
        string info;
        // Json::FastWriter fwriter;
        // Json::CharReaderBuilder reader;
        r.parse(buff, obj); // 把字符串转换成Value数据
        if (obj.isObject())
        {
            if (need == "ID")
            {
                info = obj["ID"].asString();
            }
            else if (need == "m_events")
            {
                info = obj["m_events"].asString();
            }
            else if (need == "toID")
            {
                info = obj["toID"].asString();
            }
        }
        else
        {
            return "HTTP";
        }
        return info;
    }
    int EventHandle::send_function(My_events *myev, Fdmanager::ptr fdmagr) // 发送接收到的数据
    {
        string str = myev->readBuffer->get_str();
        std::string toID = get_info(str, "toID"); // 获得要给哪个ID发信息
        int fd = fdmagr->ID_2_fd(toID);
        // string x = "me";
        // int fd = toID_2_fd(x, myev);
        if (fd > 0)
        {
            bool situtation = sendjson(fd, str); // 准备好数据和fd后进行发送
            if (situtation == true)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (fd == -1) // 还未登录
        {
            return -1;
        }
        else if (fd == 0) // 外来连接
        {
            return -2;
        }
        return -2;
    }
    bool EventHandle::send_str(int fd, string &buff)
    {
        int state = send(fd, buff.c_str(), strlen(buff.c_str()), 0);
        if (state > 0)
        {
            LY_LOG_ERROR(g_logger) << "向me, 发送了" << state << "字节";
            return true;
        }
        else if (state == -1)
        {
            LY_LOG_ERROR(g_logger) << "错误码:" << strerror(errno);
        }
        return false;
    }
    bool EventHandle::sendjson(int fd, string &buff)
    {
        Value obj;
        Reader r;

        r.parse(buff, obj); // 把字符串转换成Value数据
        if (obj.isObject())
        {
            Value s = obj["toID"];
            obj["RESPONSE"] = string(EventTypeToString(EventType::TRANSMIT_RESPOND));
            string id = s.toStyledString();
            id.erase(id.find_last_not_of("\n") + 1); // 删除末尾的换行符
            obj.removeMember("REQUEST");
            obj.removeMember("toID");          // 删除toID
            string str = obj.toStyledString(); // value转string
            int state = r_s_Msg::sendMsg(fd, str.c_str(), strlen(str.c_str()));
            if (state > 0)
            {
                LY_LOG_ERROR(g_logger) << "向" << id.c_str() << ",发送了" << state << "字节";
                return true;
            }
            else if (state == -1)
            {

                LY_LOG_ERROR(g_logger) << "错误码:" << strerror(errno);
            }
        }
        return false;
    }
    int EventHandle::RF_execute_CallBack(const Json::Value &obj, My_events *myev)
    {
        LY_LOG_INFO(g_logger) << "RF_execute_CallBack";
        rf->load_inputList(obj);
        std::vector<std::string> res = rf->execute_model();

        Json::Value res_obj;
        res_obj["RESPONSE"] = string(EventTypeToString(EventType::RF_EXECUTE_RESPONSE));

        res_obj["age"] = obj["age"].asString();
        res_obj["gender"] = obj["gender"].asString();
        res_obj["dress"] = obj["dress"].asString();
        res_obj["BMI"] = obj["BMI"].asDouble();
        res_obj["height"] = obj["height"].asDouble();
        res_obj["in_car_temp"] = obj["in_car_temp"].asDouble();
        res_obj["RH"] = obj["RH"].asDouble();
        res_obj["wind_speed"] = obj["wind_speed"].asString();

        if (obj.isMember("aircond_temp"))
        {
            res_obj["aircond_temp"] = obj["aircond_temp"].asDouble();
            res_obj["wind_m"] = obj["wind_m"].asString();
            res_obj["air_v"] = obj["air_v"].asString();
            res_obj["wind_d"] = obj["wind_d"].asString();
        }
        else
        {
            if (res[0] == "无")
            {
                res_obj["aircond_temp"] = "无";
            }
            else
            {
                res_obj["aircond_temp"] = std::stod(res[0]);
            }

            res_obj["wind_m"] = res[1];
            res_obj["air_v"] = res[2];
            res_obj["wind_d"] = res[3];
        }

        res_obj["ID"] = obj["ID"].asString();
        string str = res_obj.toStyledString();
        int fd = fdmagr->ID_2_fd("Qt");

        return r_s_Msg::sendMsg(fd, str.c_str(), strlen(str.c_str()));
        return 1;
    }
    void EventHandle::run_RF()
    {
    }
    int EventHandle::recv_dataset_one(const Json::Value &obj, My_events *myev)
    {
        std::string UserID = obj["UserID"].asString();
        std::map<std::string, std::string> dataset{
            {"age", obj["age"].asString()},
            {"gender", obj["gender"].asString()},
            {"dress", obj["dress"].asString()},
            {"wind_speed", obj["wind_speed"].asString()},
            {"BMI", obj["BMI"].asString()},
            {"height", obj["height"].asString()},
            {"RH", obj["RH"].asString()},
            {"in_car_temperature", obj["in_car_temperature"].asString()},
            {"PMV", obj["PMV"].asString()},
            {"PPD", obj["PPD"].asString()},
            {"M", obj["M"].asString()},
            {"need_temperature", obj["need_temperature"].asString()},
            {"The_wind_type", obj["The_wind_type"].asString()},
            {"wind_scale", obj["wind_scale"].asString()},
            {"direction", obj["direction"].asString()}};
        pl->insert_userid_one_dataSet(UserID, dataset);
        return 1;
    }

}
