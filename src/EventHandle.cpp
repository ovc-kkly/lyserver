#include "EventHandle.h"

namespace lyserver
{
    string EventHandle::image = "";
    EventHandle::EventHandle(/* args */)
    {
        // std::cout << "引用计数:"<<Conn_Pool.use_count() << "地址:"<<&Conn_Pool<<'\n';
        user_ = new user_db_handle;
        length = 0;
        fdmagr.reset(FdMgr::GetInstance()); // fd存储结构
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
    }
    int EventHandle::respond_client(EventType &ev, My_events* myev)
    {
        Json::Value response_info;
        response_info["RESPONSE"] = string(EventTypeToString(ev));
        string str = response_info.toStyledString();

        return r_s_Msg::sendMsg(myev->m_fd, str.c_str(), strlen(str.c_str()));
    }
    int EventHandle::ID_CallBack(const Json::Value &obj, My_events* myev)
    {
        return 0;
    }
    int EventHandle::Log_on_CallBack(const Json::Value &obj, My_events* myev)
    {
        int ret;
        EventType E;
        std::unique_ptr<user_db_handle, Deleter_resource> ptr(user_, Deleter_resource());
        int ret_ = ptr->isExistUser(obj["UserID"].asString());
        if (ret_ == 1)
        {
            if (ptr->passward_right_wrong(obj["password"].asString()))
            {
                // 密码对
                E = EventType::LOGON_RESPOND_PWRIGHT;
                ptr->change_user_log_on_state(obj["UserID"].asString(), true);
                notify_u(obj["UserID"].asString());
                ret = respond_client(E, myev);
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
            cout << "查询错误,sql语句有错或者连接池有问题" << endl;
            ret = respond_client(E, myev);
        }
        return ret;
    }
    int EventHandle::Sign_up_CallBack(const Json::Value &obj, My_events* myev)
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
            }
        }
        else if (ret_ == 1)
        {
            // 表示该用户已经存在
            E = EventType::SIGNUP_RESPOND_EXIST;
            ret = respond_client(E, myev);
        }
        else if (ret_ == -1)
        {
            // 查询失败
            E = EventType::QUERY_RESPOND_ERROR;
            cout << "查询错误,sql语句有错或者连接池有问题" << endl;
            ret = respond_client(E, myev);
        }
        return ret;
    }
    int EventHandle::Transmit_CallBack(const Json::Value &obj, My_events* myev)
    {
        return 0;
    }
    int EventHandle::Close_account_CallBack(const Json::Value &obj, My_events* myev)
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

    int EventHandle::set_client_ID(const Json::Value &obj, My_events* myev)
    {
        string ID = obj["ID"].asString();
        int st = fdmagr->whoID(myev, ID);
        if (st == 1)
        {
            printf("[身份确认,ID:%s]\n", myev->ID.c_str());
            return -1;
        }
        else if (st == -1)
        {
            printf("[身份确认失败]");
            // 清楚现在的连接

            fdmagr->free_client_resources(myev, "已经释放已经连接的", false);
            return -3;
        }
        else if (st == 0)
        {
            printf("[未知ID]");
            // 清楚现在的连接
            fdmagr->free_client_resources(myev, "已经释放已经连接的", false);
            return -3; // 未知ID应该跳出循环
        }
        else if (st == -2)
        {
            printf("[已经存在该ID]");
            // 清除现在的连接
            fdmagr->free_client_resources(myev, "已经释放已经连接的", true);
            return -3;
        }
        return -3;
    }
    int EventHandle::Log_out_CallBack(const Json::Value &obj, My_events* myev)
    {
        std::unique_ptr<user_db_handle, Deleter_resource> ptr(user_, Deleter_resource());
        ptr->change_user_log_on_state(obj["UserID"].asString(), false);
        notify_u("general");
        cout << "退出登录成功" << endl;
        return 0;
    }
    void EventHandle::notify_u(const string &userID)
    {
        string vc = "uportrait";
        int uportrait_fd = fdmagr->ID_2_fd(vc);
        if (uportrait_fd == -1)
        {
            std::cout << "uportrait未登录" << endl;
            return;
        }
        Json::Value uportrait_info;
        uportrait_info["NOTIFY"] = "USER_CHANGE";
        uportrait_info["current_userid"] = userID;
        string str = uportrait_info.toStyledString();

        r_s_Msg::sendMsg(uportrait_fd, str.c_str(), strlen(str.c_str()));
    }
    int EventHandle::imageLength_CallBack(const Json::Value &obj, My_events* myev)
    {
        // My_events *myev = (My_events *)myev_;
        int ret;
        EventType E;
        my_ocr::ocr_ptr ocrCar_ptr(new my_ocr);
        // EventHandle::ocr_sdk_ptr ptr(new aip::Ocr(this->app_id, this->api_key, this->secret_key));

        this->length = obj["length"].asInt();
        EventHandle::image.reserve(length);
        std::cout << "图片的大小：" << length << endl;
        E = EventType::READY_READ;
        Json::Value response_info;
        response_info["RESPONSE"] = string(EventTypeToString(E));
        string str = response_info.toStyledString();
        ret = send(myev->m_fd, str.c_str(), strlen(str.c_str()), 0);
        std::cout << "发送了：" << ret << std::endl;

        /*
        开始接收图片
        */
        int flags = fcntl(myev->m_fd, F_GETFL, 0);
        if (flags == -1)
        {
            perror("fcntl");
            return -1;
        }
        flags &= ~O_NONBLOCK; // 清除非阻塞标志
        if (fcntl(myev->m_fd, F_SETFL, flags) == -1)
        {
            perror("fcntl");
            return -1;
        }
        size_t count = 0;
        char buf[4098];
        while (1)
        {

            memset(buf, 0, sizeof(buf));
            int ret = recv(myev->m_fd, buf, sizeof(buf), 0);
            if (ret > 0)
            {
                count += ret;
                EventHandle::image.append(string(buf));
                std::cout << "接收到:" << ret << "累积：" << count << "总字节数：" << length << "image的数据:" << image.size() << endl;
                if (count == length)
                {
                    std::cout << "接收完毕" << endl;

                    if ((fcntl(myev->m_fd, F_SETFL, O_NONBLOCK)) < 0) // 将cfd设置为非阻塞
                    {
                        printf("%s: fcntl nonblocking failed, %s\n", __func__, strerror(errno));
                        break;
                    }
                    break;
                }
            }
            else if (ret == 0)
            {
                std::cout << "断开了" << std::endl;
            }
            else if (ret < 0)
            {

                if (errno == EAGAIN)
                {
                    std::cout << "没数据了" << endl;

                    break;
                }
            }
        }
        dete(ocrCar_ptr, image, myev->m_fd);
        return ret;
    }
    void EventHandle::dete(my_ocr::ocr_ptr &ptr, string &image, const int &fd)
    {

        std::string img = ptr->myocr_base64_decode(image);
        ptr->Recognize(img);
        std::string result = ptr->getresult();
        std::cout << result << std::endl;
        send(fd, result.c_str(), strlen(result.c_str()), 0);
        // Json::Value result;

        // result = ptr->license_plate(image, aip::null);
        // std::string color = result["words_result"]["color"].asString();
        // std::string  number= result["words_result"]["number"].asString();
        // std::cout << "颜色：" <<color <<"车牌号:"<<number <<std::endl;
        // std::string r = result.toStyledString();
        // send(fd, r.c_str(), strlen(r.c_str()), 0);
    }
    int EventHandle::recognize_CallBack(const Json::Value &obj, My_events* myev)
    {
        // My_events *myev = (My_events *)myev_;
        std::string img = obj["image"].asString();
        std::cout << img.size() << endl;

        EventHandle::image.append(obj["image"].asString());
        if (image.size() == length)
        {
            std::cout << "接收完毕" << endl;
        }
        return 1;
    }

}
